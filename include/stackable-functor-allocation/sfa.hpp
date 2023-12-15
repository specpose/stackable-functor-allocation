#pragma once
#include <array>
#include <vector>
#include <functional>

namespace NonV {
    class ROM {
        public:
        virtual void operator()() const=0;
    };
}
namespace SFA {
    template<typename T, std::size_t N, std::size_t O=N> class Strict : private std::array<T,N> {
        public:
        Strict(const std::array<T,O>& inputBuffer) : _input(inputBuffer) {}
        virtual void operator()() const=0;
	virtual std::size_t constexpr size() = 0;
        protected:
        std::array<T,O> _input;
    };
    template<typename T> class Lazy : private std::vector<T> {
        public:
        Lazy(const std::vector<T> & inputBuffer) : _input(inputBuffer){}
        virtual void operator()() const=0;
	virtual std::size_t size() = 0;
        protected:
        std::vector<T> _input;
    };
}
namespace STL {
    template<typename OutputBufferType> void function(
        typename OutputBufferType::iterator inputStart,
        typename OutputBufferType::iterator InputEnd,
        typename OutputBufferType::iterator OutputStart
        );
}
