//
// Created by fit on 16-8-27.
//

#include "ThreadPool.h"

#include "Exception.h"

#include <boost/bind.hpp>
#include <assert.h>
#include <stdio.h>

using namespace muduo;


ThreadPool::ThreadPool(const string &nameArg)
        : mutex_(),
          notEmpty_(mutex_),
          notFull_(mutex_),
          name_(nameArg),
          maxQueueSize_(0),
          running_(false) {

}

ThreadPool::~ThreadPool() {
    if (running_) {
        stop();
    }
}

void ThreadPool::start(int numThreads) {
    assert(threads_.empty());
    running_ = true;
    threads_.reserve(numThreads);
    for (int i = 0; i < numThreads; i++) {
        char id[32];
        snprintf(id, sizeof id, "%d", i + 1);
        //each thread run runInThread function, which take function from queue_ to run
        threads_.push_back(new muduo::Thread(
                boost::bind(&ThreadPool::runInThread, this), name_ + id));
        threads_[i].start();
    }
    //if numThreads equal to 0 and threadInitCallback_ is not null, run threadInitCallback_ function
    if (numThreads == 0 && threadInitCallback_) {
        threadInitCallback_();
    }
}

void ThreadPool::stop() {
    {
        MutexLockGuard lock(mutex_);
        //key code here!
        //set running_ to false, runInThread function don't take task from queue_ anymore
        running_ = false;
        ////唤醒等待queue_中元素的每个线程,其中take函数中的循环因为running_为false而暂停，立即返回一个值为空的Task对象，因为
        ////该对象为空，线程不运行任何function，直接返回　
        notEmpty_.notifyAll();
    }
    //join each thread
    //run join function of each elem of threads_
    for_each(threads_.begin(), threads_.end(), boost::bind(&muduo::Thread::join, _1));
}

size_t ThreadPool::queueSize() const {
    MutexLockGuard lock(mutex_);
    return queue_.size();
}

void ThreadPool::run(const Task &task) {
    //if threads_ size is empty, run task immediately
    if (threads_.empty()) {
        task();
    } else {
        MutexLockGuard lock(mutex_);
        //wait for not full
        while (isFull()) {
            notFull_.wait();
        }
        assert(!isFull());
        // push task function into queue_
        queue_.push_back(task);
        // now queue_ is not empty
        notEmpty_.notify();
    }
}

//右值引用版本
void ThreadPool::run(const Task &&task) {
    if (threads_.empty()) {
        task();
    } else {
        MutexLockGuard lock(mutex_);
        //wait for not full
        while (isFull()) {
            notFull_.wait();
        }
        assert(!isFull());
        // add task function into queue_
        queue_.push_back(std::move(task));
        notEmpty_.notify();
    }
}

//take function from queue_ to run it
ThreadPool::Task ThreadPool::take() {
    MutexLockGuard lock(mutex_);
    //queue_ is empty ,wait for next one
    //if not running now, just return immediately
    while (queue_.empty() && running_) {
        notEmpty_.wait();
    }
    Task task;
    if (!queue_.empty()) {
        task = queue_.front();
        queue_.pop_front();
        if (maxQueueSize_ > 0) {
            //queue_　is not full now
            notFull_.notify();
        }
    }
    return task;
}


bool ThreadPool::isFull() const {
    mutex_.assertLocked();
    return maxQueueSize_ > 0 and queue_.size() >= maxQueueSize_;
}

// main function run in each thread of thread pool
void ThreadPool::runInThread() {
    try {
        if (threadInitCallback_) {
            //before run the function, run the threadInitCallback_　function first if it exists
            threadInitCallback_();
        }
        // while running the thread
        while (running_) {
            //take function from queue_ to run it
            Task task(take());
            // if
            if (task) {
                task();
            }
        }
    } catch (const Exception &ex) {
        fprintf(stderr, "exception caught in ThreadPool %s\n", name_.c_str());
        fprintf(stderr, "reason %s\n", ex.what());
        fprintf(stderr, "stack trace: %s\n", ex.stackTrace());
        abort();
    }
}