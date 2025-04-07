#include "stackable-functor-allocation/mole.hpp"
#include <goopax>
#include <goopax_extra/struct_types.hpp>
#include <iostream>
using data_type = int;
using buffer_type = goopax::buffer<data_type>;
using resource_type = goopax::resource<data_type>;
using params_type = std::tuple<data_type>;
template<> struct Adjacent_differences<buffer_type> {
    goopax::buffer<params_type> params{};
    Adjacent_differences(goopax::goopax_device& device) : params(device, 1) {
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
template<> struct Amplify<buffer_type> {
    goopax::buffer<params_type> params{};
    Amplify(goopax::goopax_device& device, int s) : params(device, 1) {
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
    auto device = goopax::default_device(goopax::env_CPU);
    buffer_type inputData{ device, { 1,0,1,0,-1,2,3,1,0,-1,-3,-5 }};
    std::tuple<buffer_type, buffer_type> nodes{ buffer_type{},buffer_type{} };
    std::tuple<Adjacent_differences<buffer_type>, Amplify<buffer_type>> edges{ Adjacent_differences<buffer_type>(device), Amplify<buffer_type>(device,1) };
    std::get<0>(nodes) = buffer_type( device, Adjacent_differences<buffer_type>::size(inputData));
    std::get<1>(nodes) = buffer_type( device, Amplify<buffer_type>::size(std::get<0>(nodes)));
    const params_type factor{ 2 };
    std::get<1>(edges).params.copy_from_host(&factor);
    std::cout << "Sink: " << inputData.to_vector() << std::endl;
    std::get<0>(edges).forward(inputData, std::get<0>(nodes));
    std::cout << "Jammed1: " << std::get<0>(nodes).to_vector() << std::endl;
    std::get<1>(edges).forward(std::get<0>(nodes), std::get<1>(nodes));
    std::cout << "Source: " << std::get<1>(nodes).to_vector() << std::endl;

    //sync to GPU
    params_type newfactor{ 1 };
    std::get<1>(edges).params.copy_from_host(&newfactor);

    std::get<1>(edges).inverse(std::get<0>(nodes), std::get<1>(nodes));
    std::cout << "Jammed2: " << std::get<0>(nodes).to_vector() << std::endl;
    std::get<0>(edges).inverse(inputData, std::get<0>(nodes));
    std::cout << "Sink: " << inputData.to_vector() << std::endl;
}