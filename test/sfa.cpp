#include "stackable-functor-allocation/sfa.hpp"
#include <iostream>

template<typename T, std::size_t N, typename Previous> class SFAItem : public NonV::StackableFunctor<T,N,Previous> {
    public:
    SFAItem(Previous& prev) : NonV::StackableFunctor<T,N,Previous>(prev) {}
    virtual void operator()() const final {
        NonV::StackableFunctor<T,N,Previous>::_previous();
        std::cout << "Executing NonV::ROM..." << std::endl;
        //modifyMe = true;
    };
    virtual std::size_t constexpr size() final {return NonV::StackableFunctor<T,N,Previous>::_previous.size()-1;}//size of Previous?
    private:
    bool modifyMe = false;
};
template<typename T, std::size_t N, std::size_t O> class SFAFirst : public NonV::StackableFunctor<T,N,std::array<T,O>> {
    public:
    SFAFirst(std::array<T,O>& prev) : NonV::StackableFunctor<T,N,std::array<T,O>>(prev) {}
    virtual void operator()() const final {
        std::cout << "Executing NonV::ROM..." << std::endl;
        //modifyMe = true;
    };
    virtual std::size_t constexpr size() final {return O-1;}//size of Previous?
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
    virtual std::size_t constexpr size() final {return O-2;}
};

template<typename T> class FreeStore : public SFA::Lazy<T> {
    public:
    FreeStore(const std::vector<T>& inputBufferCopy) : SFA::Lazy<T>(inputBufferCopy) {}
    virtual void operator()() const final {
        std::cout << "Executing SFA::Lazy..." << std::endl;
        //this->push_back(T{});
    };
    virtual std::size_t size() final {return SFA::Lazy<T>::_input.size()-2;};
};

using data_type = int;

template<typename InputBufferType, typename OutputBufferType> void STL::transform (
        typename InputBufferType::iterator inputStart,
        typename InputBufferType::iterator inputEnd,
        typename OutputBufferType::iterator outputStart) {
        std::cout << "Executing STL::function..." << std::endl;
        std::transform(inputStart, inputEnd, outputStart,
        [](typename InputBufferType::reference a){return typename OutputBufferType::value_type{};}
        );
};

int main()
{
    const std::size_t input_length = 3;
    auto inputArray = std::array<data_type, input_length>{};
    std::cout<<"Size of inputArray "<<input_length<<std::endl;
    auto nonv = SFAFirst<data_type, input_length-1, input_length>(inputArray);
    auto nonv2 = SFAItem<data_type, input_length-2, decltype(nonv)>(nonv);
    nonv2();
    std::cout<<"Size of nonv2 "<<nonv2.size()<<std::endl;
    std::cout<<"Size of inputArray "<<input_length<<std::endl;
    auto strict = Buffer<data_type, input_length-2, input_length>(inputArray);
    strict();
    std::cout<<"Size of strict "<<strict.size()<<std::endl;
    auto inputVector = std::vector<data_type>{};
    inputVector.push_back(data_type{});
    inputVector.push_back(data_type{});
    inputVector.push_back(data_type{});
    std::cout<<"Size of inputVector "<<inputVector.size()<<std::endl;
    auto lazy = FreeStore<data_type>(inputVector);
    lazy();
    std::cout<<"Size of lazy "<<lazy.size()<<std::endl;
    auto outputBuffer = std::vector<data_type>{};
    outputBuffer.push_back(data_type{});
    std::cout<<"Size of inputVector "<<inputVector.size()<<std::endl;
    STL::transform<decltype(inputVector),decltype(outputBuffer)>(inputVector.begin(),inputVector.end(),outputBuffer.begin());
    std::cout<<"Size after STL::transform "<<outputBuffer.size()<<std::endl;
}
