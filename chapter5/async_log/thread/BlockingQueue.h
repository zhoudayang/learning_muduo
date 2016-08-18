

#ifndef BLOCKINGQUEUE_H
#define BLOCKINGQUEUE_H

#include <condition_variable>
#include <mutex>

#include <boost/noncopyable.hpp>
#include <deque>
#include <assert.h>


namespace muduo {
    template<typename T>
    class BlockingQueue : boost::noncopyable {
    public:
        BlockingQueue() : mutex_(), notEmpty_(), queue_() {}

        void put(const T &x) {
            std::unique_lock<std::mutex> lock(mutex_);
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
            return front;
        }

        size_t size() const {
            std::unique_lock<std::mutex> lock(mutex_);
            return queue_.size();
        }

    private:
        mutable std::mutex mutex_;
        std::condition_variable notEmpty_;
        std::deque<T> queue_;
    };


}


#endif //
