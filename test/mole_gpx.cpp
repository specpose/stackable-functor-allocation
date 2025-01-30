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
            return _forward(input,output,p);
            });
        inverse.assign(device, [this](goopax::resource<data_type>& input, goopax::resource<data_type>& output) {
            goopax::resource<params_type> p{params};
            return _inverse(input,output,p);
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
            return _forward(input, output, v);
            });
        inverse.assign(device, [this](goopax::resource<data_type>& input, goopax::resource<data_type>& output) {
            goopax::resource<params_type> v{ params };
            return _inverse(input, output, v);
            });
    }
    goopax::kernel<void(goopax::buffer<data_type>&, goopax::buffer<data_type>&)> forward;
    static void _forward(resource_type& input, resource_type& output, goopax::resource<params_type>& v) {
        goopax::make_gpu<std::size_t>::type factor = std::get<0>(v.front());
        goopax::gpu_for_global(*output.begin(), *output.end(), [&input, &factor](goopax::make_gpu<unsigned int>::type i) { return factor * input[i]; });//std::size_t>::type i) { return factor * input[i]; });
    }
    goopax::kernel<void(goopax::buffer<data_type>&, goopax::buffer<data_type>&)> inverse;
    static void _inverse(resource_type& input, resource_type& output, goopax::resource<params_type>& v) {
        goopax::gpu_for_global(*input.begin(), *input.end(), [&output, &v](goopax::make_gpu<unsigned int>::type i) { return output[i] / std::get<0>(v.front()); });
    }
    static std::size_t size(const buffer_type& input) { return input.size(); }
};
std::ostream& operator<<(std::ostream& os, const buffer_type& _vec) {
    std::vector<buffer_type::value_type> vec = _vec.to_vector();
    std::for_each(vec.begin(), --vec.end(), [&](typename std::vector<data_type>::value_type e) {os << e << ","; });
    os << *(--vec.end());
    return os;
}
int main() {
    auto device = goopax::default_device(goopax::env_CPU);
    std::tuple<buffer_type, buffer_type, buffer_type> nodes{ buffer_type{},buffer_type{},buffer_type{} };
    std::tuple<Adjacent_differences<buffer_type>, Amplify<buffer_type>> edges{ Adjacent_differences<buffer_type>(device), Amplify<buffer_type>(device,2) };
    std::vector<data_type> inputBuffer{ 1,0,1,0,-1,2,3,1,0,-1,-3,-5 };
    std::get<0>(nodes) = buffer_type{device, std::move(inputBuffer) };
    std::cout << "Original: " << std::get<0>(nodes) << std::endl;
    std::get<1>(nodes) = buffer_type( device, Adjacent_differences<buffer_type>::size(std::get<0>(nodes)));
    std::get<2>(nodes) = buffer_type( device, Amplify<buffer_type>::size(std::get<1>(nodes)));
    std::get<0>(edges).forward(std::get<0>(nodes), std::get<1>(nodes));
    std::get<0>(nodes).fill(0);
    std::cout << "Afterfill: " << std::get<0>(nodes) << std::endl;
    std::get<1>(edges).forward(std::get<1>(nodes), std::get<2>(nodes));

    //sync to GPU
    const params_type factor{ 1 };
    std::cout << "Amplification factor before move assignment: " << std::get<0>(factor) << std::endl;
    std::get<1>(edges).params.copy_from_host(&factor);
    params_type newfactor{ 99 };
    std::get<1>(edges).params.copy_to_host(&newfactor);
    std::cout << "Amplification factor after issuing GPU sync: " << std::get<0>(newfactor) << std::endl;

    std::get<1>(edges).inverse(std::get<1>(nodes), std::get<2>(nodes));
    std::get<0>(edges).inverse(std::get<0>(nodes), std::get<1>(nodes));
    std::cout << "Reverted: " << std::get<0>(nodes) << std::endl;
}