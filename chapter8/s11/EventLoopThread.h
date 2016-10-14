//
// Created by zhouyang on 16-9-28.
//

#ifndef S03_THREAD_ADD_TIMER_EVENTLOOPTHREAD_H
#define S03_THREAD_ADD_TIMER_EVENTLOOPTHREAD_H

#include "base/Condition.h"
#include "base/Mutex.h"
#include "base/Thread.h"

#include <boost/noncopyable.hpp>

namespace muduo {
    class EventLoop;

    class EventLoopThread : boost::noncopyable {
    public:
        EventLoopThread();

        ~EventLoopThread();

        EventLoop *startLoop();

    private:
        void threadFunc();

        EventLoop *loop_;
        bool existing_;
        Thread thread_;
        MutexLock mutex_;
        Condition cond_;
    };

}


#endif //S03_THREAD_ADD_TIMER_EVENTLOOPTHREAD_H
