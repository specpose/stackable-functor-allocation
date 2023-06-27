#include "stackable-functor-allocation/sfa.hpp"
#include <iostream>

class ROMTest : public NonV::ROM {
    public:
    constexpr void operator()() {
        //std::cout << "Executing NonV::ROM..." << std::endl;
    };
};

template<typename T, std::size_t HeapSize> class Buffer : public SFA::Strict<T,HeapSize> {
    public:
    void operator()() {
        std::cout << "Executing SFA::Strict..." << std::endl;
    };
};

template<typename T> class FreeStore : public SFA::Lazy<T> {
    public:
    void operator()() {
        std::cout << "Executing SFA::Lazy..." << std::endl;
    };
};

using data_type = int;

class ANamedBufferType : public std::vector<data_type> {};

template<> void STL::function<ANamedBufferType> (
        ANamedBufferType::iterator inputStart,
        ANamedBufferType::iterator InputEnd,
        ANamedBufferType::iterator OutputStart) {
        std::cout << "Executing STL::function..." << std::endl;
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