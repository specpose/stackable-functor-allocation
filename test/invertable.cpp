#include "stackable-functor-allocation/sfa.hpp"
#include <iostream>
#include <tuple>
#include <algorithm>
#include <cassert>

#include <vector>
using data_type = int;

template<typename buffer_t> struct Adjacent_differences_PP : public INV::ParameterPack<buffer_t> { data_type first_value = 0; };
template<typename buffer_t> struct Adjacent_differences : public INV::Invertible<buffer_t> {
    Adjacent_differences() : INV::Invertible<buffer_t>{} {}
    static void forward(Adjacent_differences_PP<buffer_t>& pack) { 
        pack.first_value = (*pack._input)[0];
        for (std::size_t i = 1; i < pack._input->size(); i++)
            (*pack._output)[i - 1] = (*pack._input)[i] - (*pack._input)[i - 1];
    }
    static void inverse(Adjacent_differences_PP<buffer_t>& pack) {
        (*pack._input)[0] = pack.first_value;
        for (std::size_t i = 0; i < pack._output->size(); i++)
            (*pack._input)[i + 1] = (*pack._input)[i] + (*pack._output)[i];
    }
    static std::size_t size(const Adjacent_differences_PP<buffer_t>& pack) {
        if (pack._input)
            return pack._input->size() - 1;
        else
            throw std::logic_error("Forgot to initialize Adj_t parameter pack");
    }
};

template<typename buffer_t> struct Amplify_PP : public INV::ParameterPack<buffer_t> { data_type factor = 2; };
template<typename buffer_t> struct Amplify : public INV::Invertible<buffer_t> {
    Amplify() : INV::Invertible<buffer_t>{} {}
    static void forward(Amplify_PP<buffer_t>& pack) {
        assert(pack._input != nullptr);
        assert(pack._output != nullptr);
        std::transform(pack._input->begin(), pack._input->end(), pack._output->begin(), [&](auto in) { return pack.factor * in; });
    }
    static void inverse(Amplify_PP<buffer_t>& pack) {
        assert(pack._input != nullptr);
        assert(pack._output != nullptr);
        std::transform(pack._output->begin(), pack._output->end(), pack._input->begin(), [&](auto out) { return out / pack.factor; });
    }
    static std::size_t size(const Amplify_PP<buffer_t>& pack) {
        if (pack._input)
            return pack._input->size();
        else
            throw std::logic_error("Forgot to initialize Amp parameter pack");
    }
};

void print_vector(std::vector<data_type> vec) {
    std::for_each(vec.begin(), vec.end(), [](data_type e) {std::cout << e << ","; });
}
int main()
{
    std::cout << "Original ";
    std::vector<data_type> inputVector{1,0,1,0,-1,2,3,1,0,-1,-3,-5};
    std::vector<data_type> outputVector(inputVector.size()-1);
    std::vector<data_type> outputVector2(inputVector.size() - 1);
    print_vector(inputVector);

    std::cout << std::endl;
    std::tuple<Adjacent_differences_PP<decltype(inputVector)>, Amplify_PP<decltype(outputVector)>> parameters{};//SVM
    std::tuple<Adjacent_differences<decltype(inputVector)>, Amplify<decltype(outputVector)>> stackable{};//kernel queue
    std::get<0>(parameters)._input = &inputVector;
    std::get<0>(parameters)._output = &outputVector;
    std::get<1>(parameters)._input = &outputVector;
    std::get<1>(parameters)._output = &outputVector2;

    std::get<0>(stackable).forward(std::get<0>(parameters));//par::unseq
    std::for_each(inputVector.begin(), inputVector.end(), [](data_type& e) { e = 0; });

    //GPU start
    std::get<1>(stackable).forward(std::get<1>(parameters));
    std::get<1>(stackable).inverse(std::get<1>(parameters));
    //GPU end
    std::get<0>(stackable).inverse(std::get<0>(parameters));//par::seq => par::unseq if input available

    std::cout << "Reverted ";
    print_vector(inputVector);
    std::cout << std::endl;
}