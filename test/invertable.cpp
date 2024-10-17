#include <iostream>
#include <vector>
#include <tuple>

namespace INV {
    template<typename output_t> struct ParameterPack {//output_t: C++
        using T = output_t;//T: Python
        ParameterPack() {}
        virtual void operator()() const = 0;
        virtual void inverse() const = 0;
        virtual std::size_t size() = 0;
        std::vector<output_t>* _output = nullptr;
        const std::vector<T>* _input;
        const ParameterPack* _params;
    };
    template<typename PPack_t> using PPack_operators
        = std::tuple< void(PPack_t::*)(), void(PPack_t::*)() >;
}
namespace INVFirst {
    template<typename output_t> struct Adjacent_differences : public INV::ParameterPack<output_t> {
        Adjacent_differences() {}
        void operator()() const { std::cout << "Hello" << std::endl; }
        void inverse() const { std::cout << "Ola! I took the first item of your inputBuffer." << std::endl; }
        std::size_t size() { return _input->size() - 1; }
    };
    //template<data_type> void Adjacent_differences::operator()(Adjacent_differences<data_type>& _params) {};
    //template<data_type> void Adjacent_differences::inverse(Adjacent_differences<data_type>& _params) {};
    template<typename output_t> using Adjacent_differences_ops = INV::PPack_operators<Adjacent_differences<typename output_t>>;
}

int main()
{
    using data_type = int;
    std::tuple<INVFirst::Adjacent_differences<data_type>> parameters{};//SHM
    std::tuple<INVFirst::Adjacent_differences_ops<data_type>> stackable{};
    const std::size_t input_length = 3;
    /*auto inputVector = std::vector<data_type>(input_length);
    auto outputVector = std::vector<data_type>(firstFunction.size());
    firstFunction._output = &outputVector;*/

    //GPU start
    //std::get<0>(std::get<0>(stackable))(std::get<0>(parameters));
    //std::get<1>(std::get<0>(stackable))(std::get<0>(parameters));
    //GPU end
}