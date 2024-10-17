#include "stackable-functor-allocation/sfa.hpp"
#include <algorithm>
#include <iostream>

template<typename T, typename Previous, int SizeChange=0> class SFAItem : public NonV::StackableFunctor<T,Previous,SizeChange> {
    public:
    SFAItem(Previous& prev) : NonV::StackableFunctor<T,Previous,SizeChange>(prev) {}
    virtual void operator()() const final {
        NonV::StackableFunctor<T,Previous,SizeChange>::_input();
        std::cout << "Executing NonV::StackableFunctor..." << std::endl;
        //modifyMe = true;
    };
    private:
    bool modifyMe = false;
};
template<typename T, std::size_t N> class SFAFirst : public std::array<T,N> {
    public:
    using std::array<T,N>::array;
    virtual void operator()() const final {
        std::cout << "Initialized NonV::Stack..." << std::endl;
    };
    constexpr static std::size_t size() {return N;};
};
template<typename T, std::size_t PreviousSize, int SizeChange=0> class Heap : public SFA::Strict<T,PreviousSize,PreviousSize+SizeChange> {
    public:
    Heap(const std::array<T,PreviousSize>& inputBufferCopy) : SFA::Strict<T,PreviousSize,PreviousSize+SizeChange>(inputBufferCopy) {}
    virtual void operator()() const final {
        std::cout << "Executing SFA::Strict..." << std::endl;
        //this->at(0)=T{};
    };
    virtual std::size_t constexpr size() final {return PreviousSize-1;}
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

template<typename InputBufferType, typename OutputBufferType> void STL::transform (
        typename InputBufferType::iterator inputStart,
        typename InputBufferType::iterator inputEnd,
        typename OutputBufferType::iterator secondStart,
        typename OutputBufferType::iterator outputStart) {
        std::cout << "Executing STL::transform..." << std::endl;
        std::transform(inputStart, inputEnd, secondStart, outputStart,
        [](typename InputBufferType::reference a,typename InputBufferType::reference b){return typename OutputBufferType::value_type{};}
        );
};

int main()
{
    const std::size_t input_length = 3;
    const int size_change = -1;
    std::cout<<"//Refactoring 4"<<std::endl;
    auto nonv1 = SFAFirst<data_type, input_length>{};
    std::cout<<"Size of nonv1 "<<std::size(nonv1)<<std::endl;
    auto nonv2 = SFAItem<data_type, decltype(nonv1), size_change>(nonv1);
    nonv2();
    std::cout<<"Size of nonv2 "<<nonv2.size()<<std::endl;
    std::cout<<"//Refactoring 3"<<std::endl;
    auto inputArray = std::array<data_type, input_length>{};
    std::cout<<"Size of inputArray "<<input_length<<std::endl;
    auto strict = Heap<data_type, input_length, size_change>(inputArray);
    strict();
    std::cout<<"Size of strict "<<strict.size()<<std::endl;
    std::cout<<"//Refactoring 2"<<std::endl;
    auto inputVector = std::vector<data_type>(input_length);
    std::cout<<"Size of inputVector "<<inputVector.size()<<std::endl;
    auto lazy = FreeStore<data_type>(inputVector);
    lazy();
    std::cout<<"Size of lazy "<<lazy.size()<<std::endl;
    std::cout<<"//Refactoring 1"<<std::endl;
    auto outputVector = std::vector<data_type>(input_length + size_change);
    std::cout<<"Size of inputVector "<<inputVector.size()<<std::endl;
    STL::transform<decltype(inputVector),decltype(outputVector)>(inputVector.begin(),inputVector.end()+size_change,inputVector.begin()-size_change,outputVector.begin());
    std::cout<<"Size of outputVector "<<outputVector.size()<<std::endl;
}
