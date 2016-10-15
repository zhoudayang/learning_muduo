//
// Created by fit on 16-10-13.
//

#ifndef S10_EVENTLOOPTHREADPOOL_H
#define S10_EVENTLOOPTHREADPOOL_H

#include "base/Condition.h"
#include "base/Mutex.h"
#include "base/Thread.h"

#include <vector>
#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

namespace muduo {

    class EventLoop;
    class EventLoopThread;

    class EventLoopThreadPool : boost::noncopyable {
    public:
        EventLoopThreadPool(EventLoop* baseLoop);

        ~EventLoopThreadPool();

        ///set the number of threads for handling input
        ///always accepts new connection in loop's thread
        ///must be called before @c start
        ///@params numThreads
        ///0 means all I/O in loop's thread, no thread will created
        ///1 means all I/O in another thread
        ///n means a thread pool with n threads, new connection
        ///are assigned on a round-robin basic

        void setThreadNum(int numThreads)
        {
            numThreads_ = numThreads;
        }

        void start();

        //TcpServer每次新建一个TcpConnection就会调用getNextLoop来取得
        //EventLoop
        EventLoop* getNextLoop();

    private:
        EventLoop* baseLoop_;
        bool started_;
        int numThreads_;
        int next_;
        boost::ptr_vector<EventLoopThread> threads_;
        std::vector<EventLoop*> loops_;
    };

}

#endif //S10_EVENTLOOPTHREADPOOL_H
