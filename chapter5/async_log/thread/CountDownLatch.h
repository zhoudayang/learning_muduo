//
// Created by fit on 16-8-18.
//

#ifndef COUNTDOWNLATCH_H
#define COUNTDOWNLATCH_H

#include <mutex>
#include <condition_variable>
#include <boost/noncopyable.hpp>

namespace muduo {
    class CountDownLatch : boost::noncopyable {
    public:
        explicit CountDownLatch(int count) : mutex_(), condition_(), count_() {}

        void wait() {
            std::unique_lock<std::mutex> lock(mutex_);
            while (count_ > 0)
                condition_.wait(lock);
        }

        void countDown() {
            std::unique_lock<std::mutex> lock(mutex_);
            --count_;
            if (count_ == 0)
                condition_.notify_all();
        }

        int getCount() const {
            std::unique_lock<std::mutex> lock(mutex_);
            return count_;
        }

    private:
        mutable std::mutex mutex_;
        std::condition_variable condition_;
        int count_;
    };


}
#endif //
