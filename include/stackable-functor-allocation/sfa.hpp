#pragma once
#include <array>
#include <vector>
#include <functional>

namespace NonV {
    template<typename T, std::size_t N, std::size_t O=N> struct RAM : public std::array<T,N> {
        public:
        RAM(const std::array<T,O>& inputBuffer) : input(inputBuffer), std::array<T,N>{} {}
        public:
        const std::array<T,O>& input;
    };
    template<typename T, std::size_t N, std::size_t O=N> class ROM {
        public:
        ROM(const RAM<T,N,O>& ioBuffer) : _input(ioBuffer) {}
        virtual void operator()() const=0;
        protected:
        const RAM<T,N,O>& _input;
    };
    //template<std::size_t M, > struct Stack : public std::array<ROM<>&,N> {};
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
namespace STL {
    template<typename InputBufferType, typename OutputBufferType> void transform(
        typename InputBufferType::iterator inputStart,
        typename InputBufferType::iterator InputEnd,
        typename OutputBufferType::iterator OutputStart
        );
}
