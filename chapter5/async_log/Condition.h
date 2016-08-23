//
// Created by fit on 16-8-19.
//

#ifndef CONDITION_H
#define CONDITION_H

#include "Mutex.h"
#include <boost/noncopyable.hpp>
#include <pthread.h>


namespace muduo {
    class Condition : boost::noncopyable {
    public:
        explicit Condition(MutexLock &mutex) : mutex_(mutex) {
            pthread_cond_init(&pcond_, NULL);
        }

        ~ Condition() {
            pthread_cond_destroy(&pcond_);
        }

        void wait() {
            MutexLock::UnassignGuard ug(mutex_);
//            ug 对象被析构时，mutex_对象的holder_就绑定了thread的tid
            pthread_cond_wait(&pcond_, mutex_.getPthreadMutex());
        }

        bool waitForSeconds(double seconds);

        void notify() {
            pthread_cond_signal(&pcond_);
        }

        void notifyAll() {
            pthread_cond_broadcast(&pcond_);
        }

    private:
        MutexLock &mutex_;
        pthread_cond_t pcond_;
    };


}


#endif
