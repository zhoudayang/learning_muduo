//
// Created by zhouyang on 16/8/1.
//

#ifndef TIMERQUEUE_H
#define TIMERQUEUE_H

#include <set>
#include <vector>

#include <boost/noncopyable.hpp>
#include "datetime/Timestamp.h"
#include "Callbacks.h"
#include "Channel.h"

namespace muduo {
    class EventLoop;

    class Timer;

    class TimerId;

    class TimerQueue : boost::noncopyable {
    public:
        TimerQueue(EventLoop *loop);

        ~TimerQueue();

        //schedules the callback to be run at given time,
        //repeats id interval >0.0
        //must be thread safe,usually be called from other threads
        TimerId addTimer(const TimerCallback &cb, Timestamp when, double interval);

    private:
        //设置key为<Timestamp,Timer *> 这样即便时间相同，他们的地址也会不同
        typedef std::pair<Timestamp, Timer *> Entry;
        typedef std::set<Entry> TimerList;

        //call when  timerfd alarms
        void handleRead();

        //move out all expired timers
        std::vector<Entry> getExpired(Timestamp now);

        void reset(const std::vector<Entry> &expired, Timestamp now);

        bool insert(Timer *timer);

        EventLoop *loop_;
        const int timerfd_;
        //timerfdChannel_
        Channel timerfdChannel_;
        //只有key 没有value
        //Timer list sorted by expiration
        TimerList timers_;

    };

}

#endif //TIMEQUEUE_TIMERQUEUE_H
