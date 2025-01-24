#include "stackable-functor-allocation/mole.hpp"
#include <tuple>
#include <goopax>
#include <iostream>
using data_type = int;
using buffer_type = goopax::buffer<data_type>;
template<> struct Adjacent_differences<buffer_type> {
    Adjacent_differences(goopax::goopax_device& device) {
        forward.assign(device,[this](goopax::resource<data_type>& input, goopax::resource<data_type>& output) {});
        inverse.assign(device, [this](goopax::resource<data_type>& input, goopax::resource<data_type>& output) {});
    }
    goopax::kernel<void(goopax::buffer<data_type>&, goopax::buffer<data_type>&)> forward;
    virtual void _forward(buffer_type& input, buffer_type& output) {
    /*    first_value = input[0];
        for (std::size_t i = 1; i < input.size(); i++)
            output.at(i - 1) = input.at(i) - input.at(i - 1);*/
    }
    goopax::kernel<void(goopax::buffer<data_type>&, goopax::buffer<data_type>&)> inverse;
    virtual void _inverse(buffer_type& input, buffer_type& output) {
    /*    input.at(0) = first_value;
        for (std::size_t i = 0; i < output.size(); i++)
            input.at(i + 1) = input.at(i) + output.at(i);*/
    }
    static std::size_t size(const buffer_type& input) { return input.size() - 1; }
    typename buffer_type::value_type first_value = 0;
};
template<> struct Amplify<buffer_type> {
    Amplify(goopax::goopax_device& device, typename buffer_type::value_type factor=1) : factor(factor) {
        forward.assign(device, [this](goopax::resource<data_type>& input, goopax::resource<data_type>& output) {});
        inverse.assign(device, [this](goopax::resource<data_type>& input, goopax::resource<data_type>& output) {});
    }
    goopax::kernel<void(goopax::buffer<data_type>&, goopax::buffer<data_type>&)> forward;
    virtual void _forward(buffer_type& input, buffer_type& output) {
        //std::transform(input.begin(), input.end(), output.begin(), [&](auto in) { return factor * in; });
    }
    goopax::kernel<void(goopax::buffer<data_type>&, goopax::buffer<data_type>&)> inverse;
    virtual void _inverse(buffer_type& input, buffer_type& output) {
        //std::transform(output.begin(), output.end(), input.begin(), [&](auto out) { return out / factor; });
    }
    static std::size_t size(const buffer_type& input) { return input.size(); }
    typename buffer_type::value_type factor;
};
std::ostream& operator<<(std::ostream& os, const buffer_type& _vec) {
    auto vec = _vec.to_vector();
    std::for_each(vec.begin(), --vec.end(), [&](typename std::vector<data_type>::value_type e) {os << e << ","; });
    os << *(--vec.end());
    return os;
}
int main() {
    auto device = goopax::default_device(goopax::env_CPU);
    std::tuple<buffer_type, buffer_type, buffer_type> nodes{};
    std::tuple<Adjacent_differences<buffer_type>, Amplify<buffer_type>> edges( {device},{device} );
    std::get<0>(nodes) = buffer_type{ device, {1,0,1,0,-1,2,3,1,0,-1,-3,-5} };
    std::cout << "Original: " << std::get<0>(nodes) << std::endl;
    std::get<1>(nodes) = buffer_type( device, Adjacent_differences<buffer_type>::size(std::get<0>(nodes)));
    std::get<2>(nodes) = buffer_type( device, Amplify<buffer_type>::size(std::get<1>(nodes)));
    std::get<0>(edges).forward(std::get<0>(nodes), std::get<1>(nodes));
    std::get<0>(nodes).fill(0);
    std::get<1>(edges).forward(std::get<1>(nodes), std::get<2>(nodes));
    std::get<1>(edges).factor = 1;
    std::get<1>(edges).inverse(std::get<1>(nodes), std::get<2>(nodes));
    std::get<0>(edges).inverse(std::get<0>(nodes), std::get<1>(nodes));
    std::cout << "Reverted: " << std::get<0>(nodes) << std::endl;
}