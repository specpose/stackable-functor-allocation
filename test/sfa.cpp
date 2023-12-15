#include "stackable-functor-allocation/sfa.hpp"
#include <iostream>

class ROMTest : public NonV::ROM {
    public:
    virtual void operator()() const final {
        std::cout << "Executing NonV::ROM..." << std::endl;
        modifyMe = true;
    };
    private:
    bool modifyMe = false;
};

template<typename T, std::size_t HeapSize> class Buffer : public SFA::Strict<T,HeapSize> {
    public:
    virtual void operator()() const final {
        std::cout << "Executing SFA::Strict..." << std::endl;
        this->at(0)=T{};
    };
};

template<typename T> class FreeStore : public SFA::Lazy<T> {
    public:
    virtual void operator()() const final {
        std::cout << "Executing SFA::Lazy..." << std::endl;
        this->push_back(T{});
    };
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
    const std::size_t heap_size = 1;
    auto nonv = ROMTest{};
    nonv();
    auto strict = Buffer<data_type, heap_size>{};
    strict();
    auto lazy = FreeStore<data_type>{};
    lazy();
    auto inputBuffer = std::vector<data_type>{};
    auto outputBuffer = ANamedBufferType{};
    STL::function<decltype(outputBuffer)>(inputBuffer.begin(),inputBuffer.end(),outputBuffer.begin());
}
