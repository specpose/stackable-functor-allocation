#include "stackable-functor-allocation/Thread.hpp"
#include <iostream>

class MyFunctor {
    public:
    void operator()(){
        std::cout << message <<std::endl;
    }
    private:
    std::string message = std::string{"Thread is running."};
};

int main()
{
    auto data = MyFunctor();
    auto t1 = SFA::Thread<MyFunctor>(data);
    t1();
}