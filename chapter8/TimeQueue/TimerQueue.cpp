//
// Created by zhouyang on 16/8/1.
//

#include "TimerQueue.h"
#include "EventLoop.h"
#include "Timer.h"
#include "TimerId.h"

#include <boost/bind.hpp>
#include <sys/timerfd.h>


namespace muduo {
    namespace detail {
        int createTimerFd() {
            int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
            if (timerfd < 0)
                printf("failed in timefd_create\n");
            return timerfd;
        }

        struct timespec howMuchTimeFromNow(Timestamp when) {
            int64_t microsecond = when.microSecondsSinceEpoch() - Timestamp::now().microSecondsSinceEpoch();
            if (microsecond < 100)
                microsecond = 100;
            struct timespec ts;
            //second
            ts.tv_sec = static_cast<time_t> (microsecond / Timestamp::kMicroSecondsPerSecond);
            //ns
            ts.tv_nsec = static_cast<long> ((microsecond % Timestamp::kMicroSecondsPerSecond) * 1000);
            return ts;

        }

        void readTimerfd(int timerfd, Timestamp now) {
            uint64_t howmany;
            ssize_t n = ::read(timerfd, &howmany, sizeof howmany);
            printf("TimerQueue::handleRead() %ld at %s \n", howmany, now.toString().c_str());
            if (n != sizeof howmany) {
                printf("TimerQueue::handleRead reads %d bytes instead of 8 \n ", n);
            }
        }

        void resetTimerfd(int timerfd, Timestamp expiration) {
            struct itimerspec newValue;
            struct itimerspec oldValue;
            bzero(&newValue, sizeof newValue);
            bzero(&oldValue, sizeof oldValue);
            newValue.it_value = howMuchTimeFromNow(expiration);
            //用来启动或者关闭fd指定的定时器
            int ret = ::timerfd_settime(timerfd, 0, &newValue, &oldValue);
            if (ret) {
                printf("timerfd_settime\n");
            }
        }
    }
}

using namespace muduo;
using namespace muduo::detail;
                                                     //create timerfd
TimerQueue::TimerQueue(EventLoop *loop) : loop_(loop), timerfd_(createTimerFd()), timerfdChannel_(loop, timerfd_),
                                          timers_() {
    timerfdChannel_.setReadCallback(boost::bind(&TimerQueue::handleRead, this));
    timerfdChannel_.enableReading();
}

TimerQueue::~TimerQueue() {
    ::close(timerfd_);
    //desctruct every elem of timers_
    for (TimerList::iterator it = timers_.begin(); it != timers_.end(); it++) {
        delete it->second;
    }
}

TimerId TimerQueue::addTimer(const TimerCallback &cb, Timestamp when, double interval) {
    Timer *timer = new Timer(cb, when, interval);
    loop_->assertInLoopThread();
    bool earliestChanged = insert(timer);
    if (earliestChanged) {
        resetTimerfd(timerfd_, timer->expiration());
    }
    return TimerId(timer);
}

void TimerQueue::handleRead() {
    loop_->assertInLoopThread();
    Timestamp now(Timestamp::now());
    readTimerfd(timerfd_, now);
    std::vector<Entry> expired = getExpired(now);
    for (std::vector<Entry>::iterator it = expired.begin(); it != expired.end(); it++) {
        //call callback function
        it->second->run();
    }
    reset(expired, now);
}

std::vector<TimerQueue::Entry> TimerQueue::getExpired(Timestamp now) {
    std::vector<Entry> expired;
    //sentry is <now,max_ptr> pair
    Entry sentry = std::make_pair(now, reinterpret_cast<Timer *>(UINTPTR_MAX));
    TimerList::iterator it = timers_.lower_bound(sentry);
    //it指向的是第一个未到期的Timer迭代器，这个迭代器指向的对象的时间必定大于now
    assert(it == timers_.end() or now < it->first);
    //将这一段已经到期的Entry复制到expired中去
    std::copy(timers_.begin(), it, std::back_inserter(expired));
    //将已经到期的Entry从timers_中删除
    timers_.erase(timers_.begin(), it);
    return expired;

}

void TimerQueue::reset(const std::vector<Entry> &expired, Timestamp now) {
    Timestamp nextExpire;
    for (std::vector<Entry>::const_iterator it = expired.begin(); it != expired.end(); it++) {
        if (it->second->repeat()) {
            //change expiration to now + interval
            it->second->restart(now);
            insert(it->second);
        }
        else {
            //don't repleat,destruct Timer
            delete it->second;
        }
    }
    if (!timers_.empty()) {
        //nextExpire time is the first elem of timers_
        nextExpire = timers_.begin()->second->expiration();
    }
    if (nextExpire.valid()) {
        resetTimerfd(timerfd_, nextExpire);
    }
}

bool TimerQueue::insert(Timer *timer) {
    bool earliestChanged = false;
    Timestamp when = timer->expiration();
    TimerList::iterator it = timers_.begin();
    if (it == timers_.end() || when < it->first) {
        earliestChanged = true;
    }
    //最早到期的timer发生了改变
    std::pair<TimerList::iterator, bool> result = timers_.insert(std::make_pair(when, timer));
    //assert result.second is not NULL
    assert(result.second);
    return earliestChanged;
}