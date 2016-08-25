//
// Created by fit on 16-8-19.
//

#ifndef MUTEX_H
#define MUTEX_H

#include <boost/noncopyable.hpp>
#include <assert.h>
#include <pthread.h>
#include "CurrentThread.h"
//pass
namespace muduo {

    class MutexLock : boost::noncopyable {
    public:
        //MutexLock constructor
        MutexLock() : holder_(0) {
            pthread_mutex_init(&mutex_, NULL);
        }

        //MutexLock destructor
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

        // lock mutex
        void lock() {
            pthread_mutex_lock(&mutex_);
            assignHolder();
        }

        //unlock mutex
        void unlock() {
            unassignHolder();
            pthread_mutex_unlock(&mutex_);
        }

        //get mutex_ of pthread_mutex_t type
        pthread_mutex_t *getPthreadMutex() {
            return &mutex_;
        }

    private:
        //设置Condition 为友元类，可以访问私有成员UnaddignGuard
        friend class Condition;

        //inner class,helper to assign holder when destruct
        class UnassignGuard : boost::noncopyable {
        public:
            UnassignGuard(MutexLock &owner) : owner_(owner) {
                owner.unassignHolder();
            }

            ~UnassignGuard() {
                owner_.assignHolder();
            }

        private:
            MutexLock &owner_;
        };

//       set holder
        void unassignHolder() {
            holder_ = 0;
        }

//      unset holder thread tid
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
        //此处必须是引用
        MutexLock &mutex_;
    };


}

//prevent misuse like
//MutexLockGuard(mutex_)
//A tempory object doesn't hold the lock for long!
#define MutexLockGuard(x) error "Missing guard object name"
#endif
