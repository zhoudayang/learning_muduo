//
// Created by fit on 16-8-23.
//

#ifndef BLOCKINGQUEUE_H
#define BLOCKINGQUEUE_H

#include <boost/noncopyable.hpp>
#include <deque>
#include <assert.h>
#include "Mutex.h"
#include "Condition.h"

//blocking queue with any size

namespace muduo {
    template<typename T>
    class BlockingQueue : boost::noncopyable {
    public:
        BlockingQueue() : mutex_(), notEmpty_(mutex_), queue_() {}

        void put(const T &x) {
            MutexLockGuard lock(mutex_);
            queue_.push_back(x);
            notEmpty_.notify();
        }

        //右值引用版本
        void put(T &&x) {
            MutexLockGuard lock(mutex_);
            queue_.push_back(std::move(x));
            notEmpty_.notify();
        }

        //take one elem from the top of the queue_
        T take() {
            MutexLockGuard lock(mutex_);
            while (queue_.empty()) {
                notEmpty_.wait();
            }
            assert(!queue_.empty());
            T front(std::move(queue_.front()));
            queue_.pop_front();
            return front;
        }

        //return the size of queue_
        size_t size() const {
            MutexLockGuard lock(mutex_);
            return queue_.size();
        }

    private:

        mutable MutexLock mutex_;
        Condition notEmpty_;
        std::deque<T> queue_;
    };


}


#endif
