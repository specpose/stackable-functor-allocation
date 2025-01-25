#include "stackable-functor-allocation/mole.hpp"
#include <tuple>
#include <goopax>
#include <iostream>
using data_type = int;
using buffer_type = goopax::buffer<data_type>;
using resource_type = goopax::resource<data_type>;
template<> struct Adjacent_differences<buffer_type> {
    Adjacent_differences(goopax::goopax_device& device) : first_value(device, 1) {
        forward.assign(device, [this](goopax::resource<data_type>& input, goopax::resource<data_type>& output) {
            goopax::resource<data_type> v1(this->first_value);
            _forward(input,output,v1);
            });
        inverse.assign(device, [this](goopax::resource<data_type>& input, goopax::resource<data_type>& output) {
            goopax::resource<data_type> v1(this->first_value);
            _inverse(input,output,v1);
            });
    }
    goopax::kernel<void(goopax::buffer<data_type>&, goopax::buffer<data_type>&)> forward;
    static void _forward(resource_type& input, resource_type& output, goopax::resource<data_type>& _first_value) {
        //first_value = input[0];
        //goopax::gpu_for_group(1, input.size() - 1, [&](goopax::gpu_uint pos) {output[pos - 1] = input[pos] - input[pos - 1]; });
        /*for (std::size_t i = 1; i < input.size(); i++)
            output.at(i - 1) = input.at(i) - input.at(i - 1);*/
    }
    goopax::kernel<void(goopax::buffer<data_type>&, goopax::buffer<data_type>&)> inverse;
    static void _inverse(resource_type& input, resource_type& output, goopax::resource<data_type>& _first_value) {
        //input[0] = first_value;
        //input[0] = 1;
        //goopax::gpu_for_group(0, output.size() - 1, [&](goopax::gpu_uint pos) {input[pos + 1] = input[pos] + output[pos]; });
        /*for (std::size_t i = 0; i < output.size(); i++)
            input.at(i + 1) = input.at(i) + output.at(i);*/
    }
    static std::size_t size(const buffer_type& input) { return input.size() - 1; }
    goopax::buffer<data_type> first_value;
};
template<> struct Amplify<buffer_type> {
    Amplify(goopax::goopax_device& device, int s) : factor(device, 1){
        factor.fill(s);
        forward.assign(device, [this](goopax::resource<data_type>& input, goopax::resource<data_type>& output) {
            goopax::resource<int> p1(this->factor);
            _forward(input, output, p1);
            });
        inverse.assign(device, [this](goopax::resource<data_type>& input, goopax::resource<data_type>& output) {
            goopax::resource<int> p1(this->factor);
            _inverse(input, output, p1);
            });
    }
    goopax::kernel<void(goopax::buffer<data_type>&, goopax::buffer<data_type>&)> forward;
    static void _forward(resource_type& input, resource_type& output, goopax::resource<int>& _factor) {
        //std::transform(input.begin(), input.end(), output.begin(), [&](auto in) { return factor * in; });
    }
    goopax::kernel<void(goopax::buffer<data_type>&, goopax::buffer<data_type>&)> inverse;
    static void _inverse(resource_type& input, resource_type& output, goopax::resource<int>& _factor) {
        //std::transform(output.begin(), output.end(), input.begin(), [&](auto out) { return out / factor; });
    }
    static std::size_t size(const buffer_type& input) { return input.size(); }
    goopax::buffer<int> factor;
};
std::ostream& operator<<(std::ostream& os, const buffer_type& _vec) {
    auto vec = _vec.to_vector();
    std::for_each(vec.begin(), --vec.end(), [&](typename std::vector<data_type>::value_type e) {os << e << ","; });
    os << *(--vec.end());
    return os;
}
int main() {
    auto device = goopax::default_device(goopax::env_CPU);
    std::tuple<buffer_type, buffer_type, buffer_type> nodes{
        buffer_type{ device, {1,0,1,0,-1,2,3,1,0,-1,-3,-5} },
        buffer_type{ device, 11},
        buffer_type{ device, 11}
    };
    Adjacent_differences<buffer_type> test1{ device };
    Amplify<buffer_type> test2{ device,2 };
    /*std::tuple<Adjacent_differences<buffer_type>, Amplify<buffer_type>> edges( {device},{device,2} );
    std::get<0>(nodes) = buffer_type{ device, {1,0,1,0,-1,2,3,1,0,-1,-3,-5} };
    std::cout << "Original: " << std::get<0>(nodes) << std::endl;
    std::get<1>(nodes) = buffer_type( device, Adjacent_differences<buffer_type>::size(std::get<0>(nodes)));
    std::get<2>(nodes) = buffer_type( device, Amplify<buffer_type>::size(std::get<1>(nodes)));
    std::get<0>(edges).forward(std::get<0>(nodes), std::get<1>(nodes));
    std::get<0>(nodes).fill(0);
    std::get<1>(edges).forward(std::get<1>(nodes), std::get<2>(nodes));
    //std::get<1>(edges).factor = 1;
    std::get<1>(edges).inverse(std::get<1>(nodes), std::get<2>(nodes));
    std::get<0>(edges).inverse(std::get<0>(nodes), std::get<1>(nodes));
    std::cout << "Reverted: " << std::get<0>(nodes) << std::endl;*/
}