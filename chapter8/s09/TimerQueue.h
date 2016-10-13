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

//
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

        // void cancel(TimerId timerId);

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

    };

}


#endif //S02_TIMERQUEUE_H
