#include <iostream>
#include <vector>

namespace INV {
    struct ParameterPack {};
    template<typename output_t> class Lazy {//output_t: C++
    public:
        using T = output_t;//T: Python
        Lazy(const std::vector<T>& inputBuffer, const ParameterPack& functionParameters) 
            : _input(inputBuffer), _params(functionParameters) {}
        virtual void operator()() const = 0;
        virtual void inverse() const = 0;
        virtual std::size_t size() = 0;
        std::vector<output_t>* _output = nullptr;
    protected:
        const std::vector<T>& _input;
        const ParameterPack& _params;
    };
}
using data_type = int;
struct INVFirstPack : INV::ParameterPack {};
class INVFirst : public INV::Lazy<data_type> {
public:
    INVFirst(const std::vector<data_type>& inputBuffer, const INVFirstPack& functionParameters) 
        : INV::Lazy<data_type>(inputBuffer, functionParameters) {}
    void operator()() const { std::cout << "Hello" << std::endl; }
    void inverse() const { std::cout << "Ola!" << std::endl; }
    std::size_t size() { return _input.size() - 1; }
};
int main()
{
    INVFirstPack firstFunctionParameters{};//SHM
    const std::size_t input_length = 3;
    auto inputVector = std::vector<data_type>(input_length);
    auto firstFunction = INVFirst(inputVector, firstFunctionParameters);//GPU
    auto outputVector = std::vector<data_type>(firstFunction.size());
    firstFunction._output = &outputVector;

    //GPU start
    firstFunction();
    firstFunction.inverse();
    //GPU end
}