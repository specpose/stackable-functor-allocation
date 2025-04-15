#include "stackable-functor-allocation/mole.hpp"
#include <goopax>
#include <goopax_extra/struct_types.hpp>
#include <iostream>
using data_type = int;
using buffer_type = goopax::buffer<data_type>;
using resource_type = goopax::resource<data_type>;
using params_type = std::tuple<data_type>;
static goopax::goopax_device device = goopax::default_device(goopax::env_CPU);
template<> struct MOLE::Node<buffer_type> {
    using input_type = buffer_type;
    using output_type = input_type;
    Node(
        buffer_type& input,
        std::function<std::size_t(buffer_type&)> f = [](buffer_type& input) -> std::size_t { return input.size(); }
    ) : _input(input), _output(device, f(input)) {}
    buffer_type& _input;
    buffer_type _output;
};
template<> struct Adjacent_differences<buffer_type> : public MOLE::Node<buffer_type>{
    goopax::buffer<params_type> params{};
    Adjacent_differences(buffer_type& input) : MOLE::Node<buffer_type>(input, [](buffer_type& input) -> std::size_t { return input.size() - 1; }), params(device, 1) {
        forward.assign(device, [this]() {
            goopax::resource<data_type> input{_input};
            goopax::resource<data_type> output{_output};
            goopax::resource<params_type> p{params};
            _forward(input,output,p);
            });
        inverse.assign(device, [this]() {
            goopax::resource<data_type> input{_input};
            goopax::resource<data_type> output{_output};
            goopax::resource<params_type> p{params};
            _inverse(input,output,p);
            });
    }
    goopax::kernel<void()> forward;
    static void _forward(resource_type& input, resource_type& output, goopax::resource<params_type>& p) {
        std::get<0>(p.front()) = input[0];
        goopax::make_gpu<unsigned int>::type size = input.size();
        goopax::gpu_for<std::less<>>(1, size, 1, [&](goopax::make_gpu<unsigned int>::type pos)
        {
            output[pos - 1] = input[pos] - input[pos - 1];
        });
    }
    goopax::kernel<void()> inverse;
    static void _inverse(resource_type& input, resource_type& output, goopax::resource<params_type>& p) {
        input[0] = std::get<0>(p.front());
        goopax::make_gpu<unsigned int>::type size = output.size();
        goopax::gpu_for<std::less<>>(0, size, 1,[&](goopax::make_gpu<unsigned int>::type pos)//not explicit unrolling?
        {
            input[pos + 1] = input[pos] + output[pos];
        });
    }
};
template<> struct Amplify<buffer_type> : public MOLE::Node<buffer_type>{
    goopax::buffer<params_type> params{};
    Amplify(buffer_type& input, int s=1) : MOLE::Node<buffer_type>(input), params(device, 1) {
        const std::vector<params_type> myParams{ { s } };
        params = std::move(myParams);
        forward.assign(device, [this]() {
            goopax::resource<data_type> input{_input};
            goopax::resource<data_type> output{_output};
            goopax::resource<params_type> v{ params };
            _forward(input, output, v);
            });
        inverse.assign(device, [this]() {
            goopax::resource<data_type> input{_input};
            goopax::resource<data_type> output{_output};
            goopax::resource<params_type> v{ params };
            _inverse(input, output, v);
            });
    }
    goopax::kernel<void()> forward;
    static void _forward(resource_type& input, resource_type& output, goopax::resource<params_type>& v) {
        goopax::gpu_for_global(0, output.size(), [&input, &output, &v](goopax::make_gpu<unsigned int>::type i) { output[i] = std::get<0>(v.front()) * input[i]; });
    }
    goopax::kernel<void()> inverse;
    static void _inverse(resource_type& input, resource_type& output, goopax::resource<params_type>& v) {
        goopax::gpu_for_global(0, input.size(), [&output, &input, &v](goopax::make_gpu<unsigned int>::type i) { input[i] = output[i] / std::get<0>(v.front()); });
    }
    void setFactor(data_type s) {
        const params_type factor{ s };
        params.copy_from_host(&factor);
    }
};
std::ostream& operator<<(std::ostream& os, const std::vector<data_type> &vec) {
    std::for_each(vec.begin(), --vec.end(), [&](typename std::vector<data_type>::value_type e) {os << e << ","; });
    os << *(--vec.end());
    return os;
}
int main() {
    std::vector<data_type> vec{ 1,0,1,0,-1,2,3,1,0,-1,-3,-5 };
    buffer_type inputData{ device, std::move(vec)};
    MOLE::Stack<Adjacent_differences<buffer_type>, Amplify<buffer_type>> edges(inputData);
    MOLE::get<1>(edges).setFactor(2);
    std::cout << "Stack size is " << std::tuple_size<std::tuple<Adjacent_differences<buffer_type>, Amplify<buffer_type>>>{} << std::endl;
    std::cout << "Sink: " << inputData.to_vector() << std::endl;
    MOLE::get<0>(edges).forward();
    std::cout << "Jammed1: " << MOLE::get<0>(edges)._output.to_vector() << std::endl;
    MOLE::get<1>(edges).forward();
    std::cout << "Source: " << MOLE::get<1>(edges)._output.to_vector() << std::endl;

    //sync to GPU
    MOLE::get<1>(edges).setFactor(1);

    MOLE::get<1>(edges).inverse();
    std::cout << "Jammed2: " << MOLE::get<0>(edges)._output.to_vector() << std::endl;
    MOLE::get<0>(edges).inverse();
    std::cout << "Sink: " << inputData.to_vector() << std::endl;
}