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
//template<typename functor_t > MOLE::Stack<functor_t>::Stack(goopax::goopax_device& device, typename functor_t::input_type& input, std::size_t index = 0) : _myself(device,input) {};
//template<typename functor_t, typename... other_functors> MOLE::Stack<functor_t, other_functors...>::Stack(goopax::goopax:device& device, typename functor_t::input_type& input, std::size_t index=0) : _myself(device,input), _index(index), _others(_myself._output,++_index) {};
template<> struct Adjacent_differences<buffer_type> : public MOLE::Node<buffer_type>{
    goopax::buffer<params_type> params{};
    Adjacent_differences(buffer_type& input) : MOLE::Node<buffer_type>(input), params(device, 1) {
        forward.assign(device, [this](goopax::resource<data_type>& input, goopax::resource<data_type>& output) {
            goopax::resource<params_type> p{params};
            _forward(input,output,p);
            });
        inverse.assign(device, [this](goopax::resource<data_type>& input, goopax::resource<data_type>& output) {
            goopax::resource<params_type> p{params};
            _inverse(input,output,p);
            });
    }
    goopax::kernel<void(goopax::buffer<data_type>&, goopax::buffer<data_type>&)> forward;
    static void _forward(resource_type& input, resource_type& output, goopax::resource<params_type>& p) {
        std::get<0>(p.front()) = input[0];
        goopax::make_gpu<unsigned int>::type size = input.size();
        for (unsigned int pos = 1; pos < 13; pos++)//size; pos++)
        {
            output[pos - 1] = input[pos] - input[pos - 1];
        };
    }
    goopax::kernel<void(goopax::buffer<data_type>&, goopax::buffer<data_type>&)> inverse;
    static void _inverse(resource_type& input, resource_type& output, goopax::resource<params_type>& p) {
        input[0] = std::get<0>(p.front());
        for (unsigned int pos = 0; pos < 12; pos++)//Single ALU?
        {
            input[pos + 1] = input[pos] + output[pos];
        };
    }
    static std::size_t size(const buffer_type& input) { return input.size() - 1; }
};
template<> struct Amplify<buffer_type> : public MOLE::Node<buffer_type>{
    goopax::buffer<params_type> params{};
    Amplify(buffer_type& input, int s=1) : MOLE::Node<buffer_type>(input), params(device, 1) {
        const std::vector<params_type> myParams{ { s } };
        params = std::move(myParams);
        forward.assign(device, [this](goopax::resource<data_type>& input, goopax::resource<data_type>& output) {
            goopax::resource<params_type> v{ params };
            _forward(input, output, v);
            });
        inverse.assign(device, [this](goopax::resource<data_type>& input, goopax::resource<data_type>& output) {
            goopax::resource<params_type> v{ params };
            _inverse(input, output, v);
            });
    }
    goopax::kernel<void(goopax::buffer<data_type>&, goopax::buffer<data_type>&)> forward;
    static void _forward(resource_type& input, resource_type& output, goopax::resource<params_type>& v) {
        goopax::gpu_for_global(0, output.size(), [&input, &output, &v](goopax::make_gpu<unsigned int>::type i) { output[i] = std::get<0>(v.front()) * input[i]; });
    }
    goopax::kernel<void(goopax::buffer<data_type>&, goopax::buffer<data_type>&)> inverse;
    static void _inverse(resource_type& input, resource_type& output, goopax::resource<params_type>& v) {
        goopax::gpu_for_global(0, input.size(), [&output, &input, &v](goopax::make_gpu<unsigned int>::type i) { input[i] = output[i] / std::get<0>(v.front()); });
    }
    static std::size_t size(const buffer_type& input) { return input.size(); }
};
std::ostream& operator<<(std::ostream& os, const std::vector<data_type> &vec) {
    std::for_each(vec.begin(), --vec.end(), [&](typename std::vector<data_type>::value_type e) {os << e << ","; });
    os << *(--vec.end());
    return os;
}
int main() {
    buffer_type inputData{ device, { 1,0,1,0,-1,2,3,1,0,-1,-3,-5 }};
    MOLE::Stack<Adjacent_differences<buffer_type>, Amplify<buffer_type>> edges(inputData);
    const params_type factor{ 2 };
    MOLE::get<1>(edges).params.copy_from_host(&factor);
    std::cout << "Sink: " << inputData.to_vector() << std::endl;
    MOLE::get<0>(edges).forward(MOLE::get<0>(edges)._input, MOLE::get<0>(edges)._output);
    std::cout << "Jammed1: " << MOLE::get<0>(edges)._output.to_vector() << std::endl;
    MOLE::get<1>(edges).forward(MOLE::get<1>(edges)._input, MOLE::get<1>(edges)._output);
    std::cout << "Source: " << MOLE::get<1>(edges)._output.to_vector() << std::endl;

    //sync to GPU
    params_type newfactor{ 1 };
    MOLE::get<1>(edges).params.copy_from_host(&newfactor);

    MOLE::get<1>(edges).inverse(MOLE::get<1>(edges)._input, MOLE::get<1>(edges)._output);
    std::cout << "Jammed2: " << MOLE::get<0>(edges)._output.to_vector() << std::endl;
    MOLE::get<0>(edges).inverse(MOLE::get<0>(edges)._input, MOLE::get<0>(edges)._output);
    std::cout << "Sink: " << inputData.to_vector() << std::endl;
}