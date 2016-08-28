//
// Created by fit on 16-8-23.
//

#ifndef COUNTDOWNLATCH_H
#define COUNTDOWNLATCH_H

#include "Condition.h"
#include "Mutex.h"

namespace muduo {
    class CountDownLatch : boost::noncopyable {
    public:
        explicit CountDownLatch(int count);

        //wait for count_ to become zero
        void wait();

        //decrease the count_
        void countDown();

        //return count_
        int getCount() const;

    private:
        mutable MutexLock mutex_;
        Condition condition_;
        int count_;
    };


}


#endif //
