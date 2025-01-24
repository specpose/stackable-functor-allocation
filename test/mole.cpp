#include "stackable-functor-allocation/mole.hpp"
#include <tuple>
#include <vector>
#include <algorithm>
#include <iostream>
//Use n nodes and n-1 bidirectional edges => Not SFA!
template<typename container_t> struct Invertible {
    Invertible() {}
    virtual void forward(container_t& input, container_t& output) = 0;
    virtual void inverse(container_t& input, container_t& output) = 0;
    static std::size_t size(const container_t& input) = delete;
};
template<typename container_t> struct Adjacent_differences : public Invertible<container_t> {
    Adjacent_differences() : Invertible<container_t>{} {}
    virtual void forward(container_t& input, container_t& output) {
        first_value = input.at(0);
        for (std::size_t i = 1; i < input.size(); i++)
            output.at(i - 1) = input.at(i) - input.at(i - 1);
    }
    virtual void inverse(container_t& input, container_t& output) {
        input.at(0) = first_value;
        for (std::size_t i = 0; i < output.size(); i++)
            input.at(i + 1) = input.at(i) + output.at(i);
    }
    static std::size_t size(const container_t& input) { return input.size() - 1; }
    typename container_t::value_type first_value = 0;
};
template<typename container_t> struct Amplify : public Invertible<container_t> {
    Amplify() : Invertible<container_t>{} {}
    Amplify(typename container_t::value_type factor) : factor(factor) {}
    virtual void forward(container_t& input, container_t& output) {
        std::transform(input.begin(), input.end(), output.begin(), [&](auto in) { return factor * in; });
    }
    virtual void inverse(container_t& input, container_t& output) {
        std::transform(output.begin(), output.end(), input.begin(), [&](auto out) { return out / factor; });
    }
    static std::size_t size(const container_t& input) { return input.size(); }
    typename container_t::value_type factor = 1;
};
using data_type = int;
using buffer_type = std::vector<data_type>;
std::ostream& operator<<(std::ostream& os, const buffer_type& vec) {
    std::for_each(vec.begin(), --vec.end(), [&](typename buffer_type::value_type e) {os << e << ","; });
    os << *(--vec.end());
    return os;
}
int main() {
	std::tuple<buffer_type, buffer_type, buffer_type> nodes{};
    std::tuple<Adjacent_differences<buffer_type>, Amplify<buffer_type>> edges( {},{2} );
    std::get<0>(nodes) = buffer_type{ 1,0,1,0,-1,2,3,1,0,-1,-3,-5 };
    std::cout << "Original: " << std::get<0>(nodes) << std::endl;
    std::get<1>(nodes) = buffer_type(Adjacent_differences<buffer_type>::size(std::get<0>(nodes)));
    std::get<2>(nodes) = buffer_type(Amplify<buffer_type>::size(std::get<1>(nodes)));
    std::get<0>(edges).forward(std::get<0>(nodes), std::get<1>(nodes));
    std::for_each(std::get<0>(nodes).begin(), std::get<0>(nodes).end(), [](data_type& e) { e = 0; });
    std::get<1>(edges).forward(std::get<1>(nodes), std::get<2>(nodes));
    std::get<1>(edges).factor = 1;
    std::get<1>(edges).inverse(std::get<1>(nodes), std::get<2>(nodes));
    std::get<0>(edges).inverse(std::get<0>(nodes), std::get<1>(nodes));
    std::cout << "Reverted: " << std::get<0>(nodes) << std::endl;
}