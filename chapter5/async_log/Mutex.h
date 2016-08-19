//
// Created by fit on 16-8-19.
//

#ifndef MUTEX_H
#define MUTEX_H

#include <boost/noncopyable.hpp>
#include <CurrentThread.h>

#ifdef NDEBUG
#define NDEBUG
#include <assert.h>
#endif

#include <pthread.h>

namespace muduo {

    class MutexLock : boost::noncopyable {
    public:
        MutexLock() : holder_(0) {
            pthread_mutex_init(&mutex_, NULL);
        }

        ~MutexLock() {
            assert(holder_ == 0);
            pthread_mutex_destroy(&mutex_);
        }

        bool isLockedByThisThread() const {
            return holder_ == CurrentThread::tid();
        }

        void assertLocked() const {
            assert(isLockedByThisThread());
        }

        void lock() {
            pthread_mutex_lock(&mutex_);
            assignHolder();
        }

        void unlock() {
            unassignHolder();
            pthread_mutex_unlock(&mutex_);
        }

        pthread_mutex_t *getPthreadMutex() {
            return &mutex_;
        }

    private:
        friend class Condition;

        void unassignHolder() {
            holder_ = 0;
        }

        void assignHolder() {
            holder_ = CurrentThread::tid();
        }

        pthread_mutex_t mutex_;
        pid_t holder_;
    };


    class MutexLockGuard : boost::noncopyable {
    public:
        explicit MutexLockGuard(MutexLock &mutex) : mutex_(mutex) {
            mutex_.lock();
        }

        ~MutexLockGuard() {
            mutex_.unlock();
        }

    private:
        MutexLock &mutex_;
    };


}


#endif
