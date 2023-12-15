#pragma once
#include <array>
#include <vector>
#include <functional>

namespace NonV {
    class ROM {
        public:
        virtual void const operator()()=0;
    };
}
namespace SFA {
    template<typename T, std::size_t N> class Strict : public std::array<T,N> {
        public:
        virtual void const operator()()=0;
	virtual std::size_t constexpr size() final {return N;};
    };
    template<typename T> class Lazy : public std::vector<T> {
        public:
        virtual void const operator()()=0;
	virtual std::size_t size() final {return this->size();};
    };
}
namespace STL {
    template<typename OutputBufferType> void function(
        typename OutputBufferType::iterator inputStart,
        typename OutputBufferType::iterator InputEnd,
        typename OutputBufferType::iterator OutputStart
        );
}
