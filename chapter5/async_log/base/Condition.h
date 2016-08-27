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
        //constructor
        explicit Condition(MutexLock &mutex) : mutex_(mutex) {
            pthread_cond_init(&pcond_, NULL);
        }

        //destructor
        ~ Condition() {
            pthread_cond_destroy(&pcond_);
        }

        //wait for condition to notify
        void wait() {
            MutexLock::UnassignGuard ug(mutex_);
//            ug 对象被析构时，mutex_对象的holder_就绑定了thread的tid
            pthread_cond_wait(&pcond_, mutex_.getPthreadMutex());
        }

        //wait for several seconds
        bool waitForSeconds(double seconds);

        //notify one random thread
        void notify() {
            pthread_cond_signal(&pcond_);
        }

        //notify all thread
        void notifyAll() {
            pthread_cond_broadcast(&pcond_);
        }

    private:
        //此处必须为引用
        MutexLock &mutex_;
        pthread_cond_t pcond_;
    };


}


#endif
