//
// Created by zhouyang on 16-9-28.
//

#ifndef S02_TIMERQUEUE_H
#define S02_TIMERQUEUE_H

#include <set>
#include <vector>

#include <boost/noncopyable.hpp>

#include "base/Timestamp.h"
#include "base/Mutex.h"
#include "Callback.h"
#include "Channel.h"

using std::shared_ptr;

namespace muduo {

    class EventLoop;

    class Timer;

    class TimerId;

/// A best efforts timer queue.
/// No guarantee that the callback will be on time.
///
    class TimerQueue : boost::noncopyable {
    public:
        TimerQueue(EventLoop *loop);

        ~TimerQueue();

        ///
        /// Schedules the callback to be run at given time,
        /// repeats if @c interval > 0.0.
        ///
        /// Must be thread safe. Usually be called from other threads.
        TimerId addTimer(const TimerCallback &cb,
                         Timestamp when,
                         double interval);

        void cancel(TimerId timerId);

    private:

        typedef std::pair<Timestamp, Timer *> Entry;
        //按照触发时间的先后顺序存储Entry
        typedef std::set<Entry> TimerList;

        // called when timerfd alarms
        void handleRead();

        // move out all expired timers
        std::vector<Entry> getExpired(Timestamp now);

        void reset(const std::vector<Entry> &expired, Timestamp now);

        bool insert(Timer *timer);

        EventLoop *loop_;
        const int timerfd_;
        Channel timerfdChannel_;
        // Timer list sorted by expiration
        TimerList timers_;

        void addTimerInLoop(Timer *timer);

        void cancelInLoop(TimerId timerId);

        typedef std::pair<Timer* ,int64_t> ActiveTimer;
        typedef std::set<ActiveTimer> ActiveTimerSet;


        ActiveTimerSet activeTimers_;
        //这两个变量用于应付自注销这种情况，即在定时器中回调注销当前定时器的情况
        bool callingExpiredTimers_;
        //记录自行删除Timer的那些Timer
        ActiveTimerSet cancelingTimers_;

    };

}


#endif //S02_TIMERQUEUE_H
