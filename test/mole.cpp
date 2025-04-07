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
    buffer_type inputData{ 1, 0, 1, 0, -1, 2, 3, 1, 0, -1, -3, -5 };
    MOLE::Stack<Adjacent_differences<buffer_type>, Amplify<buffer_type>> edges(inputData);
    std::get<0>(MOLE::get<1>(edges).params) = 2;
    std::cout << "Stack size is " << std::tuple_size<std::tuple<Adjacent_differences<buffer_type>, Amplify<buffer_type>>>{} << std::endl;
    std::cout << "Sink: " << MOLE::get<0>(edges)._input << std::endl;
    MOLE::get<0>(edges).forward(MOLE::get<0>(edges)._input, MOLE::get<0>(edges)._output, MOLE::get<0>(edges).params);
    std::cout << "Jammed1: " << MOLE::get<0>(edges)._output << std::endl;
    std::fill(MOLE::get<0>(edges)._input.begin(), MOLE::get<0>(edges)._input.end(), 0);
    MOLE::get<1>(edges).forward(MOLE::get<1>(edges)._input, MOLE::get<1>(edges)._output, MOLE::get<1>(edges).params);
    std::cout << "Source: " << MOLE::get<1>(edges)._output << std::endl;

    //sync to GPU
    std::get<0>(MOLE::get<1>(edges).params) = 1;

    MOLE::get<1>(edges).inverse(MOLE::get<1>(edges)._input, MOLE::get<1>(edges)._output, MOLE::get<1>(edges).params);
    std::cout << "Jammed2: " << MOLE::get<1>(edges)._input << std::endl;
    MOLE::get<0>(edges).inverse(MOLE::get<0>(edges)._input, MOLE::get<0>(edges)._output, MOLE::get<0>(edges).params);
    std::cout << "Sink: " << MOLE::get<0>(edges)._input << std::endl;
}
