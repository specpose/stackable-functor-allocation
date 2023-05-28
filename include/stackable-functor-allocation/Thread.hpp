#pragma once
#include "stackable-functor-allocation/error.h"
#include <thread>
#include <functional>

namespace SFA {
    template<typename T> class Thread {
        public:
	    Thread(T functor) : functorCopy(functor){
            if (_thread){
#if DEBUG
                throw SFA::util::runtime_error("Constructor: Thread is a leftover thread",__FILE__,__func__);
#endif
            } else {
                _running=true;
                _thread = new std::thread( std::mem_fn(&T::operator()),functorCopy );
            }
        };
        virtual ~Thread() {
            if (_thread) {
#if DEBUG
                throw SFA::util::runtime_error("Destructor: Thread is still running",__FILE__,__func__);
#endif
                this->join();
            }
        };
        virtual void operator()() final{
            this->join();
        };
        private:
        void join(){
            if (_thread){
                if (_thread->joinable()){
                    _thread->join();
                }
#if DEBUG
                if (_running || _thread->joinable())//still running?
				    throw SFA::util::runtime_error("SuperClass has not set running to false",__FILE__,__func__);
#endif
			    delete _thread;
			    _thread = nullptr;
            }
        };
        bool _running=false;
        std::thread* _thread=nullptr;
        T functorCopy;
    };
}
