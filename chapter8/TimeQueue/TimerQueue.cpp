//
// Created by zhouyang on 16/8/1.
//



#include "TimerQueue.h"
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
            ts.tv_sec = static_cast<time_t> (microsecond / Timestamp::kMicroSecondsPerSecond);
            ts.tv_nsec = static_cast<long> (microsecond % Timestamp::kMicroSecondsPerSecond);
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
            int ret = ::timerfd_settime(timerfd, 0, &newValue, &oldValue);
            if (ret) {
                printf("timerfd_settime\n");

            }

        }
    }
}

using namespace muduo;
using namespace muduo::detail;

TimerQueue::TimerQueue(EventLoop *loop):loop_(loop),timerfd_(createTimerFd()),timerfdChannel_(loop,timerfd_) ,timers_(){
    timerfdChannel_.setReadCallback(boost::bind(&TimerQueue::handleRead,this));
    timerfdChannel_.eableReading();
}

TimerQueue::~TimerQueue() {
    ::close(timerfd_);
    for(TimerList::iterator it = timers_.begin();it != timers_.end();it++){
        delete it->second;
    }
}

TimerId TimerQueue::addTimer(const TimerCallback &cb, Timestamp when, double interval) {
    Timer * timer = new Timer(cb,when,interval);
    loop_->assertInLoopThread();
    bool earliestChanged = insert(timer);
    if(earliestChanged){
        resetTimerfd(timerfd_,timer->expiration());
    }
    return TimerId(timer);
}

void TimerQueue::handleRead() {
    loop_->assertInLoopThread();
    Timestamp now(Timestamp::now());
    readTimerfd(timerfd_,now);
    std::vector<Entry> expired = getExpired(now);
    for(std::vector<Entry>::iterator it = expired.begin();it != expired.end();it++){
        it->second->run();
    }
    reset(expired,now);
}

std::vector <TimerQueue::Entry> TimerQueue::getExpired(Timestamp now) {
    std::vector <Entry> expired;
    Entry sentry = std::make_pair(now, reinterpret_cast<Timer *>(UINTPTR_MAX));
    TimerList ::iterator it = timers_.lower_bound(sentry);
    assert(it==timers_.end() or now<it->first);
    std::copy(timers_.begin(),it,std::back_inserter(expired));
    timers_.erase(timers_.begin(),it);
    return expired;

}

void TimerQueue::reset(const std::vector<Entry> &expired, Timestamp now) {
    Timestamp nextExpire;
    for(std::vector<Entry>::const_iterator it = expired.begin();it!=expired.end();it++){
        if(it->second->repeat()){
            it->second->restart(now);
            insert(it->second);
        }
        else{
            delete it->second;
        }
    }
    if(!timers_.empty()){
        nextExpire=timers_.begin()->second->expiration();
    }
    if(nextExpire.valid()){
        resetTimerfd(timerfd_,nextExpire);
    }
}

bool TimerQueue::insert(Timer *timer) {
    bool earliestChanged = false;
    Timestamp when = timer->expiration();
    TimerList ::iterator it = timers_.begin();
    if(it==timers_.end()||when<it->first){
        earliestChanged=true;
    }
    std::pair<TimerList::iterator,bool> result = timers_.insert(std::make_pair(when,timer));
    assert(result.second);
    return earliestChanged;
}