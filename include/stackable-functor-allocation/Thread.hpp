#pragma once

namespace SFA {
    struct Strict {
        protected:
        virtual void constexpr operator()()=0;
    };
    struct Lazy {
        protected:
        virtual void operator()()=0;
    };
}
