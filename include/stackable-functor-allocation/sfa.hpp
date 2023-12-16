#pragma once
#include <array>
#include <vector>
#include <functional>
namespace STL {
    template<typename InputBufferType, typename OutputBufferType> void transform(
        typename InputBufferType::iterator inputStart,
        typename InputBufferType::iterator InputEnd,
        typename OutputBufferType::iterator OutputStart
        );
}
namespace SFA {
    template<typename T, std::size_t N, std::size_t O=N> class Strict : public std::array<T,N> {
        public:
        Strict(const std::array<T,O>& inputBuffer) : _input(inputBuffer), std::array<T,N>{} {}
        virtual void operator()() const=0;//nonstatic!
	virtual std::size_t constexpr size() = 0;
        protected:
        std::array<T,O> _input;
    };
    template<typename T> class Lazy : public std::vector<T> {
        public:
        Lazy(const std::vector<T> & inputBuffer) : _input(inputBuffer), std::vector<T>{} {}
        virtual void operator()() const=0;//nonstatic
	virtual std::size_t size() = 0;
        protected:
        std::vector<T> _input;
    };
}
namespace NonV {
    template<typename T, std::size_t N, typename Previous> class StackableFunctor : public virtual std::array<T,N> {
        public:
        StackableFunctor(Previous& prev) : _previous(prev), std::array<T,N>{} {}
        virtual void operator()() const=0;//nonstatic
        virtual std::size_t constexpr size() = 0;
        protected:
        Previous& _previous;
    };
}
