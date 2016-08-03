//
// Created by zhouyang on 16-8-3.
//

#ifndef EVENTLOOPTHREAD_H
#define EVENTLOOPTHREAD_H
#include <mutex>
#include <condition_variable>
#include "Thread.h"

#include <boost/noncopyable.hpp>

using std::mutex;
using std::condition_variable;

namespace muduo{
    class EventLoop;
    class EventLoopThread:boost::noncopyable{
    public:
        EventLoopThread();
        ~EventLoopThread();
        EventLoop * startLoop();
    private:
        void threadFunc();
        EventLoop *loop_;
        bool exiting_;
        Thread thread_;
        mutex mutex_;
        condition_variable cond_;
    };

}

#endif //TIMEQUEUE_EVENTLOOPTHREAD_H
