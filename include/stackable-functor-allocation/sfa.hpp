#pragma once
#include <array>
#include <vector>

namespace NonV {
    class ROM {
        public:
        virtual void constexpr operator()()=0;
    };
}
namespace SFA {
    template<typename T, std::size_t N> class Strict : public std::array<T,N> {
        public:
        virtual void operator()()=0;
    };
    template<typename T> class Lazy : public std::vector<T> {
        public:
        virtual void operator()()=0;
    };
}
namespace STL {
    template<typename OutputBufferType> void function(
        typename OutputBufferType::iterator inputStart,
        typename OutputBufferType::iterator InputEnd,
        typename OutputBufferType::iterator OutputStart
        );
}