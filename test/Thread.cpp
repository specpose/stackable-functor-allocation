#include "stackable-functor-allocation/Thread.hpp"
#include <iostream>

class ThreadImpl : public SFA::Thread {
    public:
    void operator()(){
        _running = true;
            std::cout << "Thread is running."<<std::endl;
        _running = false;
    }
    ~ThreadImpl() {
	    join();
    }
    bool isRunning(){
	    return _running;
    }
    private:
    bool _running = false;
};

int main()
{
    auto t1 = ThreadImpl();
    t1();
    t1.stop();
}