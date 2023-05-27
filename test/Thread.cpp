#include "stackable-functor-allocation/Thread.hpp"
#include <iostream>

class MyFunctor {
    public:
    void operator()(){
        std::cout << "Thread is running."<<std::endl;
    }
};

int main()
{
    auto data = MyFunctor();
    auto t1 = SFA::Thread<MyFunctor>(data);
    t1();
}