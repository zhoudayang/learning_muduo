//
// Created by zhouyang on 16-9-28.
//

#include "TimerQueue.h"

#include "base/Logging.h"
#include "EventLoop.h"
#include "Timer.h"
#include "TimerId.h"

#include <boost/bind.hpp>
#include <boost/foreach.hpp>

#include <sys/timerfd.h>

namespace muduo {
    namespace detail {
        int createTimerfd() {
            /* Monotonic system-wide clock.  */
            int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
            if (timerfd < 0)
            {
                LOG_SYSFATAL << "Failer in timerfd_create";
            }
            return timerfd;
        }

        //get how much time from now, return as struct timespec
        struct timespec howMuchTimeFromNow(Timestamp when) {
            int64_t microseconds = when.microSecondsSinceEpoch() - Timestamp::now().microSecondsSinceEpoch();
            if (microseconds < 100)
            {
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
            if (n != sizeof howmany)
            {
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
            if (ret)
            {
                LOG_SYSERR << "timerfd_settime()";
            }
        }
    }
}

using namespace muduo;
using namespace muduo::detail;

//小结：当触发定时器时会拿出当前超时的所有Entry,调用其run函数(即run callback function)。调用完毕会将需要重复执行的Entry reset之后加入TimerList,并且设置定时器下一次触发时间。
//增加新的定时任务时，看这次添加是否会改变第一次超时的Entry，若改变，则重新设置定时器超时时间。
TimerQueue::TimerQueue(EventLoop *loop)
        : loop_(loop),
          timerfd_(createTimerfd()),
        //init a new timer channel
          timerfdChannel_(loop, timerfd_),
          timers_(),
          callingExpiredTimers_(false)
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
         it != timers_.end(); ++it)
    {
        delete it->second;
    }
}


// 这里是题眼
void TimerQueue::handleRead() {
    loop_->assertInLoopThread();
    Timestamp now(Timestamp::now());
    //read from timer
    readTimerfd(timerfd_, now);
    //get timeout Entry
    std::vector<Entry> expired = getExpired(now);
    // 如果当前正在运行getExpired对应的timer，下面要开始reset,为了避免cancel的loop被重新加入，要看这个Timer是否在cancelingTimers_中
    callingExpiredTimers_ = true;
    //首先将上一次记录的需要cancel的容易cancelingTimers_清空
    cancelingTimers_.clear();
    // safe to callback outside critical section
    for (std::vector<Entry>::iterator it = expired.begin();
         it != expired.end(); ++it)
    {
        //run callback function
        it->second->run();
    }
    callingExpiredTimers_ = false;
    //处理那些需要重复执行的timer,将他们重新加入TimerList
    reset(expired, now);
}

std::vector<TimerQueue::Entry> TimerQueue::getExpired(Timestamp now) {
    assert(timers_.size() == activeTimers_.size());

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
    //boost foreach do same thing for each element in container
    //从activeTimers_中删除对应的timer
    BOOST_FOREACH(Entry entry,expired){
        ActiveTimer timer (entry.second,entry.second->sequence());
        size_t n = activeTimers_.erase(timer);
        assert(n==1);
    }
    assert(timers_.size() == activeTimers_.size());
    return expired;
}

void TimerQueue::reset(const std::vector<Entry> &expired, Timestamp now) {
    Timestamp nextExpire;

    for (std::vector<Entry>::const_iterator it = expired.begin();
         it != expired.end(); ++it)
    {
        ///add below these 2 line code to support cancel
        ///如果当前正在运行getExpired对应的timer，下面要开始reset,为了避免cancel的loop被重新加入，要看这个Timer是否在cancelingTimers_中
        ActiveTimer timer(it->second,it->second->sequence());
        if (it->second->repeat() && cancelingTimers_.find(timer) == cancelingTimers_.end())
        {
            it->second->restart(now);
            //处理重复定时器
            insert(it->second);
        }
        else
        {
            delete it->second;
        }
    }

    if (!timers_.empty())
    {
        nextExpire = timers_.begin()->second->expiration();
    }
    //原始的nextExpire如果不经过上述if分支，is not valid
    //if nextExpire time is valid, reset timer next alarm time
    if (nextExpire.valid())
    {
        resetTimerfd(timerfd_, nextExpire);
    }
}

bool TimerQueue::insert(Timer *timer) {
    loop_->assertInLoopThread();
    assert(timers_.size() == activeTimers_.size());
    bool earliestChanged = false;
    Timestamp when = timer->expiration();
    TimerList::iterator it = timers_.begin();
    if (it == timers_.end() || when < it->first)
    {
        earliestChanged = true;
    }
    // 将新的timer同时插入timers_ 和 activeTimers_
    {
        std::pair<TimerList::iterator, bool> result =
                timers_.insert(std::make_pair(when, timer));
        //assert insert successful
        assert(result.second);
    }
    {
        std::pair<ActiveTimerSet::iterator,bool> result = activeTimers_.insert(ActiveTimer(timer,timer->sequence()));
        assert(result.second);
    }
    assert(timers_.size() == activeTimers_.size());
    return earliestChanged;
}

TimerId TimerQueue::addTimer(const TimerCallback &cb, Timestamp when, double interval) {
    Timer *timer = new Timer(cb, when, interval);
    //call addTimerInLoop function to add timer, to ensure add timer in io thread
    loop_->runInLoop(boost::bind(&TimerQueue::addTimerInLoop, this, timer));
    return TimerId(timer,timer->sequence());
}

void TimerQueue::addTimerInLoop(Timer *timer) {
    loop_->assertInLoopThread();
    bool earliestChanged = insert(timer);
    //如果最先触发的时刻有变，重新设置Timer fd
    if (earliestChanged)
    {
        resetTimerfd(timerfd_, timer->expiration());
    }
}


//add function to support timer

void TimerQueue::cancel(TimerId timerId) {
    loop_->runInLoop(boost::bind(&TimerQueue::cancelInLoop,this,timerId));
}


void TimerQueue::cancelInLoop(TimerId timerId) {
    loop_->assertInLoopThread();
    assert(timers_.size() == activeTimers_.size());
    ActiveTimer timer(timerId.timer_,timerId.sequence_);
    ActiveTimerSet::iterator it = activeTimers_.find(timer);
    if(it!=activeTimers_.end()){
        size_t n = timers_.erase(Entry(it->first->expiration(),it->first));
        assert(n==1);
        delete it->first;//fixme::no delete please
        activeTimers_.erase(it);
    }
        // 如果当前正在运行getExpired对应的timer，下面要开始reset,为了避免cancel的loop被重新加入，要看这个Timer是否在cancelingTimers_中
    else if(callingExpiredTimers_){
        cancelingTimers_.insert(timer);
    }
    assert(timers_.size() == activeTimers_.size());
}


