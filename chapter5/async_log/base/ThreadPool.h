//
// Created by fit on 16-8-27.
//

#ifndef ASYNC_LOG_THREADPOOL_H
#define ASYNC_LOG_THREADPOOL_H

#include "Condition.h"
#include "Mutex.h"
#include "Thread.h"
#include "Types.h"

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

#include <deque>

namespace muduo {
    class ThreadPool : boost::noncopyable {
    public:
        typedef boost::function<void()> Task;

        explicit ThreadPool(const string &nameArg = string("ThreadPool"));

        ~ThreadPool();

        //set thread pool max size
        void setMaxQueueSize(int maxSize) {
            maxQueueSize_ = maxSize;
        }

        //set thread pool init callback function
        void setThreadInitCallback(const Task &cb) {
            threadInitCallback_ = cb;
        }

        //start run thread pool
        void start(int numThreads);

        //stop every function of thread pool
        void stop();

        //return thread pool name
        const string &name() const {
            return name_;
        }

        size_t queueSize() const;

        void run(const Task &f);

        void run(const Task && f);

    private:
        bool isFull() const;

        void runInThread();

        Task take();

        mutable MutexLock mutex_;
        Condition notEmpty_;
        Condition notFull_;
        string name_;
        Task threadInitCallback_;
        boost::ptr_vector<muduo::Thread> threads_;
        std::deque<Task> queue_;
        size_t maxQueueSize_;
        bool running_;
    };

}


#endif //ASYNC_LOG_THREADPOOL_H
