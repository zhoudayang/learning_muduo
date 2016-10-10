//
// Created by zhouyang on 16-9-28.
//

#ifndef S02_TIMER_H
#define S02_TIMER_H

#include <boost/noncopyable.hpp>

#include "base/Timestamp.h"
#include "Callback.h"

namespace muduo {

    class Timer : boost::noncopyable {
    public:
        //callback function, expire time, run interval, if repeat?
        Timer(const TimerCallback &cb, Timestamp when, double interval) :
                callback_(cb),
                expiration_(when),
                interval_(interval),
                repeat_(interval > 0.0) {

        }

        void run() const {
            //run callback function
            callback_();
        }

        //return expire time
        Timestamp expiration() const {
            return expiration_;
        }

        //if repeat ?
        bool repeat() const {
            return repeat_;
        }

        //if repeat, reset expiration_ to next expire time pos, else set expiration_ to invalid Timestamp
        void restart(Timestamp now);

    private:
        //timer callback function
        const TimerCallback callback_;
        //expire time
        Timestamp expiration_;
        //run interval
        const double interval_;
        //if repeat ?
        const bool repeat_;
    };
}

#endif //S02_TIMER_H
