#pragma once
#include "stackable-functor-allocation/error.h"
#include <thread>
#include <functional>

namespace SFA {
    class Thread {
        public:
	    Thread() : _thread(nullptr){};
        virtual ~Thread() {
#if DEBUG
            if (_thread)
                throw SFA::util::runtime_error("Destructor: Thread is still running",__FILE__,__func__);
#endif
        };
        virtual void join(){
            if (_thread){
                if (_thread->joinable()){
                    _thread->join();
                }
#if DEBUG
                if (isRunning() || _thread->joinable())//still running?
				    throw SFA::util::runtime_error("SuperClass has not set running to false",__FILE__,__func__);
#endif
			    delete _thread;
			    _thread = nullptr;
                }
        }
        virtual void stop() {
		    join();
	    };
        std::thread::id get_id(){
            if (_thread){
                return _thread->get_id();
            } else {
#if DEBUG
                throw SFA::util::runtime_error("Thread does not exist yet",__FILE__,__func__);
#endif
            }
	    }
        virtual void run(){
            if (_thread){
#if DEBUG
                throw SFA::util::runtime_error("Constructor: Thread is a leftover thread",__FILE__,__func__);
#endif
            } else {
                _thread = new std::thread( std::mem_fn(&Thread::operator()),this );
            }
            //_thread->swap(t2);
        };
        protected:
        virtual void operator()()=0;
        virtual bool isRunning()=0;
        private:
        std::thread* _thread;
    };
}
