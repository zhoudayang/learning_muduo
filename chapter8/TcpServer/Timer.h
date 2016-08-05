//
// Created by zhouyang on 16/8/1.
//

#ifndef TIMER_H
#define TIMER_H

#include <boost/noncopyable.hpp>
#include "datetime/Timestamp.h"
#include "Callbacks.h"

namespace muduo {
    class Timer : boost::noncopyable {
    public:
        //if interval >0.0 means run every interval
        Timer(const TimerCallback &cb, Timestamp when, double interval)
                : callback_(cb), expiration_(when), interval_(interval), repeat_(interval_ > 0.0) {

        }

        //runs callback function
        void run() const {
            callback_();
        }

        //return timestamp when timer expire to alarm
        Timestamp expiration() const {
            return expiration_;
        }

        //repeat after timer alarms ?
        bool repeat() const {
            return repeat_;
        }

        //reset expiration_, called when repeat
        void restart(Timestamp now);

    private:
        //callback function
        const TimerCallback callback_;
        //timestamp when timer should alarm
        Timestamp expiration_;
        //repeat interval
        const double interval_;
        //if should repeat when timer alarm
        const bool repeat_;
    };


}


#endif
