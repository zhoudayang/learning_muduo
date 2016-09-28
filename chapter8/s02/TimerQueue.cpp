//
// Created by zhouyang on 16-9-28.
//

#include "TimerQueue.h"

#include "base/Logging.h"
#include "EventLoop.h"
#include "Timer.h"
#include "TimerId.h"

#include <boost/bind.hpp>

#include <sys/timerfd.h>

namespace muduo {
    namespace detail {
        int createTimerfd() {
            int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
            if (timerfd < 0) {
                LOG_SYSFATAL << "Failer in timerfd_create";
            }
            return timerfd;
        }

        //get how much time from now, return as struct timespec
        struct timespec howMuchTimeFromNow(Timestamp when) {
            int64_t microseconds = when.microSecondsSinceEpoch() - Timestamp::now().microSecondsSinceEpoch();
            if (microseconds < 100) {
                microseconds = 100;
            }
            struct timespec ts;
            ts.tv_sec = static_cast<time_t>(microseconds / Timestamp::kMicroSecondsPerSecond);
            // Nanoseconds.  transform from microseconds to nanoseconds
            ts.tv_nsec = static_cast<long>((microseconds % Timestamp::kMicroSecondsPerSecond) * 1000);
            return ts;
        }

        //read event that occurred in timer fd
        void readTimerfd(int timerfd, Timestamp now) {
            uint64_t howmany;
            ssize_t n = ::read(timerfd, &howmany, sizeof howmany);
            LOG_TRACE << "TimerQueue::handleRead() " << howmany << " at " << now.toString();
            if (n != sizeof howmany) {
                LOG_ERROR << "TimerQueue::handleRead() reads " << n << "bytes instead of 8 ";
            }
        }

        //重新设置定时器的超时时间
        void resetTimerfd(int timerfd, Timestamp expiration) {
            struct itimerspec newValue;
            struct itimerspec oldValue;
            bzero(&newValue, sizeof newValue);
            bzero(&oldValue, sizeof oldValue);
            newValue.it_value = howMuchTimeFromNow(expiration);
            int ret = ::timerfd_settime(timerfd, 0, &newValue, &oldValue);
            /*
                int timerfd_settime(int fd, int flags, const struct itimerspec *new_value, struct itimerspec *old_value);

                作用：用来启动或关闭有fd指定的定时器

                参数：

                fd：timerfd，有timerfd_create函数返回

                fnew_value:指定新的超时时间，设定new_value.it_value非零则启动定时器，否则关闭定时器，如果new_value.it_interval为0，则定时器只定时一次，即初始那次，否则之后每隔设定时间超时一次

                old_value：不为null，则返回定时器这次设置之前的超时时间

                flags：1代表设置的是绝对时间；为0代表相对时间。
             */
            if (ret) {
                LOG_SYSERR << "timerfd_settime()";
            }
        }
    }
}

using namespace muduo;
using namespace muduo::detail;

TimerQueue::TimerQueue(EventLoop *loop)
        : loop_(loop),
          timerfd_(createTimerfd()),
          //init a new timer channel
          timerfdChannel_(loop, timerfd_),
          timers_()
{
    //set read callback function
    timerfdChannel_.setReadCallback(boost::bind(&TimerQueue::handleRead, this));
    // we are always reading the timerfd, we disarm it with timerfd_settime.
    //enable reading
    timerfdChannel_.enableReading();
}

TimerQueue::~TimerQueue() {
    ::close(timerfd_);
    // do not remove channel, since we're in EventLoop::dtor();
    for (TimerList::iterator it = timers_.begin();
         it != timers_.end(); ++it) {
        delete it->second;
    }
}

TimerId TimerQueue::addTimer(const TimerCallback &cb, Timestamp when, double interval) {
    Timer *timer = new Timer(cb, when, interval);
    loop_->assertInLoopThread();
    //timer是否最先超时
    bool earliestChanged = insert(timer);

    if (earliestChanged) {
        //reset timer
        resetTimerfd(timerfd_, timer->expiration());
    }
    return TimerId(timer);
}

// 这里是题眼
void TimerQueue::handleRead() {
    loop_->assertInLoopThread();
    Timestamp now(Timestamp::now());
    //read from timer
    readTimerfd(timerfd_, now);
    //get timeout Entry
    std::vector<Entry> expired = getExpired(now);

    // safe to callback outside critical section
    for (std::vector<Entry>::iterator it = expired.begin();
         it != expired.end(); ++it) {
        //run callback function
        it->second->run();
    }
    //处理那些需要重复执行的timer,将他们重新加入TimerList
    reset(expired, now);
}

std::vector<TimerQueue::Entry> TimerQueue::getExpired(Timestamp now) {
    std::vector<Entry> expired;
    //用当前时间和最大的地址值来作为查找的分隔点
    Entry sentry = std::make_pair(now, reinterpret_cast<Timer *>(UINTPTR_MAX));
    //lower_bound 返回大于或等于val的第一个元素位置，相对应的upper_bound 返回的是大于或者等于val的最后一个元素的位置
    TimerList::iterator it = timers_.lower_bound(sentry);
    /// 这里返回的是第一个未到期的Timer的迭代器，所以断言应该是 now < it->first
    assert(it == timers_.end() || now < it->first);
    //将begin到it的所有元素复制到expired中去
    std::copy(timers_.begin(), it, back_inserter(expired));
    //将这些处理过的元素删除　
    timers_.erase(timers_.begin(), it);
    return expired;
}

void TimerQueue::reset(const std::vector<Entry> &expired, Timestamp now) {
    Timestamp nextExpire;

    for (std::vector<Entry>::const_iterator it = expired.begin();
         it != expired.end(); ++it) {
        if (it->second->repeat()) {
            it->second->restart(now);
            //处理重复定时器
            insert(it->second);
        }
        else {
            delete it->second;
        }
    }

    if (!timers_.empty()) {
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
    std::pair<TimerList::iterator, bool> result =
            timers_.insert(std::make_pair(when, timer));
    assert(result.second);
    return earliestChanged;
}

