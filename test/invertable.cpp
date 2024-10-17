#include <iostream>
#include <vector>
#include <tuple>

namespace INV {
    template<typename output_t> struct ParameterPack {//output_t: C++
        using input_t = output_t;//input_t: Python
        ParameterPack() {}
        ~ParameterPack() { _input = nullptr; _output = nullptr; }
        const std::vector<output_t>* _output = nullptr;
        const std::vector<input_t>* _input = nullptr;
    };
    template<typename output_t> struct Invertable {
        Invertable() {}
        static void forward(ParameterPack<output_t>&) = delete;
        static void inverse(ParameterPack<output_t>&) = delete;
        static std::size_t size(ParameterPack<output_t>&) = delete;
    };
}
template<typename output_t> struct Adjacent_differences_PP : public INV::ParameterPack<output_t> { std::string name{ "Chico" }; };
template<typename output_t> struct Adjacent_differences : public INV::Invertable<output_t> {
    Adjacent_differences() : INV::Invertable<output_t>{} {}
    static void forward(Adjacent_differences_PP<output_t>& pack) { std::cout << "Hello" << std::endl; }
    static void inverse(Adjacent_differences_PP<output_t>& pack) { std::cout << "Ola! "<<pack.name<<" I took the first item of your inputBuffer." << std::endl; }
    static std::size_t size(Adjacent_differences_PP<output_t>& pack) {
        if (pack._input)
            return pack._input->size() - 1;
        else
            throw std::logic_error("Forgot to initialize Adj_t parameter pack");
    }
};

int main()
{
    using data_type = int;
    std::tuple<Adjacent_differences_PP<data_type>> parameters{};//SHM
    std::tuple<Adjacent_differences<data_type>> stackable{};//kernel stack
    const std::size_t input_length = 3;
    auto inputVector = std::vector<data_type>(input_length);
    std::get<0>(parameters)._input = &inputVector;
    auto outputVector = std::vector<data_type>(std::get<0>(stackable).size(std::get<0>(parameters)));
    std::get<0>(parameters)._output = &outputVector;

    //GPU start
    std::get<0>(stackable).forward(std::get<0>(parameters));//par::unseq
    std::get<0>(stackable).inverse(std::get<0>(parameters));//par::seq => par::unseq if input available
    //GPU end
}