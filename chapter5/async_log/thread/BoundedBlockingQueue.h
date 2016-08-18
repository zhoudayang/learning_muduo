//
// Created by fit on 16-8-18.
//

#ifndef BOUNDEDBLOCKINGQUEUE_H
#define BOUNDEDBLOCKINGQUEUE_H

#include <condition_variable>
#include <mutex>

#include <boost/circular_buffer.hpp>
#include <boost/noncopyable.hpp>
#include <assert.h>
//有界阻塞队列
namespace muduo {
    template<typename T>
    class BoundedBlockingQueue : boost::noncopyable {
    public:
        explicit BoundedBlockingQueue(int maxSize) : mutex_(), notEmpty_(), notFull_(), queue_(maxSize) {}

        void put(const T &x) {
            std::unique_lock<std::mutex> lock(mutex_);
            while (queue_.full()) {
                notFull_.wait(lock);
            }
            assert(!queue_.full());
            queue_.push_back(x);
            notEmpty_.notify_one();
        }

        T take() {
            std::unique_lock<std::mutex> lock(mutex_);
            while (queue_.empty()) {
                notEmpty_.wait(lock);
            }
            assert(!queue_.empty());
            T front(queue_.front());
            queue_.pop_front();
            notFull_.notify_one();
            return front;
        }

        bool empty() const {
            std::unique_lock<std::mutex> lock(mutex_);
            return queue_.size();
        }

        bool full() const {
            std::unique_lock<std::mutex> lock(mutex_);
            return queue_.full();
        }

        size_t size() const {
            std::unique_lock<std::mutex> lock(mutex_);
            return queue_.size();
        }

        size_t capacity() const {
            std::unique_lock<std::mutex> lock(mutex_);
            return queue_.capacity();
        }

    private:
        mutable std::mutex mutex_;
        std::condition_variable notEmpty_;
        std::condition_variable notFull_;
        boost::circular_buffer<T> queue_;
    };
}

#endif //
