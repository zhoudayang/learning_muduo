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
//线程池

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

        //return current size of thread pool
        size_t queueSize() const;

        //add function to thread pool
        void run(const Task &f);

        //add function to thread pool 右值引用版本
        void run(const Task &&f);

    private:
        //thread pool is full now ?
        bool isFull() const;

        //run task function in thread
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
        // if running_ is false, 唤醒take函数，他会立即返回
        bool running_;
    };

}


#endif //ASYNC_LOG_THREADPOOL_H
