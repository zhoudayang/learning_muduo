//
// Created by fit on 16-8-2.
//

#include "Timer.h"

using namespace muduo;

void Timer::restart(Timestamp now) {
    if (repeat_) {
        //init new Timestamp as now +interval_
        expiration_ = addTime(now, interval_);
    }
    else
        expiration_ = Timestamp::invalid();
}