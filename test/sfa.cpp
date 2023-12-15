#include "stackable-functor-allocation/sfa.hpp"
#include <iostream>

class ROMTest : public NonV::ROM {
    public:
    virtual void operator()() const final {
        std::cout << "Executing NonV::ROM..." << std::endl;
        //modifyMe = true;
    };
    private:
    bool modifyMe = false;
};

template<typename T, std::size_t N, std::size_t O=N> class Buffer : public SFA::Strict<T,N,O> {
    public:
    Buffer(const std::array<T,O>& inputBufferCopy) : SFA::Strict<T,N,O>(inputBufferCopy) {}
    virtual void operator()() const final {
        std::cout << "Executing SFA::Strict..." << std::endl;
        //this->at(0)=T{};
    };
    virtual std::size_t constexpr size() final {return O-1;}
};

template<typename T> class FreeStore : public SFA::Lazy<T> {
    public:
    FreeStore(const std::vector<T>& inputBufferCopy) : SFA::Lazy<T>(inputBufferCopy) {}
    virtual void operator()() const final {
        std::cout << "Executing SFA::Lazy..." << std::endl;
        //this->push_back(T{});
    };
    virtual std::size_t size() final {return SFA::Lazy<T>::_input.size()-1;};
};

using data_type = int;

class ANamedBufferType : public std::vector<data_type> {};

template<> void STL::function<ANamedBufferType> (
        ANamedBufferType::iterator inputStart,
        ANamedBufferType::iterator inputEnd,
        ANamedBufferType::iterator outputStart) {
        std::cout << "Executing STL::function..." << std::endl;
        std::transform(inputStart, inputEnd, outputStart,[](ANamedBufferType::reference a){return ANamedBufferType::value_type{};});
};

int main()
{
    const std::size_t input_length = 2;
    auto nonv = ROMTest{};
    nonv();
    auto inputArray = std::array<data_type,input_length>{};
    auto strict = Buffer<data_type, input_length>(inputArray);
    strict();
    auto inputVector = std::vector<data_type>{};
    auto lazy = FreeStore<data_type>(inputVector);
    lazy();
    auto outputBuffer = ANamedBufferType{};
    STL::function<decltype(outputBuffer)>(inputVector.begin(),inputVector.end(),outputBuffer.begin());
}
