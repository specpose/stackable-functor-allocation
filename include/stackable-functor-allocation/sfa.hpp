#pragma once
#include <array>
#include <vector>
#include <functional>
namespace STL {
    template<typename InputBufferType, typename OutputBufferType> void transform(//Refactoring 1
        typename InputBufferType::iterator inputStart,
        typename InputBufferType::iterator inputEnd,
        typename OutputBufferType::iterator secondStart,
        typename OutputBufferType::iterator outputStart
    );
}
namespace SFA {
    template<typename T> class Lazy : public std::vector<T> {//Refactoring 2
        public:
        Lazy(const std::vector<T> & inputBuffer) : _input(inputBuffer), std::vector<T>{} {}
        virtual void operator()() const=0;//nonstatic!
	virtual std::size_t size() = 0;
        protected:
        std::vector<T> _input;
    };
    template<typename T, std::size_t PreviousSize, int SizeChange> class Strict : public std::array<T,PreviousSize+SizeChange> {//Refactoring 3
        public:
        Strict(const std::array<T,PreviousSize>& inputBuffer) : _input(inputBuffer), std::array<T,PreviousSize+SizeChange>{} {}
        virtual void operator()() const=0;//nonstatic!
	virtual std::size_t constexpr size() = 0;
        protected:
        std::array<T,PreviousSize> _input;
    };
}
namespace NonV {
    template<typename T, typename Previous, int SizeChange, size_t N=Previous::size()+SizeChange> class StackableFunctor : public std::array<T,N> {//Refactoring 4
        public:
        StackableFunctor(Previous& prev) : _input(prev), std::array<T,N>{} {}
        virtual void operator()() const=0;//nonstatic!
        constexpr static std::size_t size() {return N;};
        protected:
        Previous _input;
    };
}

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
