#include "stackable-functor-allocation/mole.hpp"
#include <vector>
#include <iostream>
using data_type = int;
using buffer_type = std::vector<data_type>;
std::ostream& operator<<(std::ostream& os, const buffer_type& vec) {
    std::for_each(vec.begin(), --vec.end(), [&](typename buffer_type::value_type e) {os << e << ","; });
    os << *(--vec.end());
    return os;
}
int main() {
    std::tuple<buffer_type, buffer_type, buffer_type> nodes{ buffer_type{},buffer_type{},buffer_type{} };
    std::tuple<Adjacent_differences<buffer_type>, Amplify<buffer_type>> edges{ Adjacent_differences<buffer_type>(), Amplify<buffer_type>(2) };
    std::get<0>(nodes) = buffer_type{ 1,0,1,0,-1,2,3,1,0,-1,-3,-5 };
    std::cout << "Sink: " << std::get<0>(nodes) << std::endl;
    std::get<1>(nodes) = buffer_type(Adjacent_differences<buffer_type>::size(std::get<0>(nodes)));
    std::get<2>(nodes) = buffer_type(Amplify<buffer_type>::size(std::get<1>(nodes)));
    std::get<0>(edges).forward(std::get<0>(nodes), std::get<1>(nodes), std::get<0>(edges).params);
    std::cout << "Jammed1: " << std::get<1>(nodes) << std::endl;
    std::fill(std::get<0>(nodes).begin(), std::get<0>(nodes).end(), 0);
    std::get<1>(edges).forward(std::get<1>(nodes), std::get<2>(nodes), std::get<1>(edges).params);
    std::cout << "Source: " << std::get<2>(nodes) << std::endl;
    std::get<0>(std::get<1>(edges).params) = 1;
    //sync to GPU
    std::get<1>(edges).inverse(std::get<1>(nodes), std::get<2>(nodes), std::get<1>(edges).params);
    std::cout << "Jammed2: " << std::get<1>(nodes) << std::endl;
    std::get<0>(edges).inverse(std::get<0>(nodes), std::get<1>(nodes), std::get<0>(edges).params);
    std::cout << "Sink: " << std::get<0>(nodes) << std::endl;
}
