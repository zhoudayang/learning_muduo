//
// Created by fit on 16-8-19.
//

#include "Condition.h"
#include <errno.h>


bool muduo::Condition::waitForSeconds(double seconds) {
    struct timespec abstime;
    /*
     * struct timespec {
	    long	tv_sec; // second
	    long    tv_nsec;// nano second
       }
     */
    //获取现在的实时时间
    clock_gettime(CLOCK_REALTIME, &abstime);
    // 1e9 nano second equal to one second
    const int64_t kNanoSecondsPerSecond = 1e9;
    int64_t nanoseconds = static_cast<int64_t>(seconds * kNanoSecondsPerSecond);
    //timespec
    abstime.tv_sec += static_cast<time_t>((abstime.tv_nsec + nanoseconds) / kNanoSecondsPerSecond);
    abstime.tv_nsec = static_cast<long>((abstime.tv_nsec + nanoseconds) % kNanoSecondsPerSecond);
    //destruct to call unassign function
    MutexLock::UnassignGuard ug(mutex_);
    //Connection timed out
    //For a shared condition variable, the time specified by abstime has passed. the function return ETIMEDOUT
    return ETIMEDOUT == pthread_cond_timedwait(&pcond_, mutex_.getPthreadMutex(), &abstime);
}