//
// Created by zhouyang on 16-9-28.
//

#include "Timer.h"

using namespace muduo;

//init static 成员
AtomicInt64 Timer::s_numCreated_;

void Timer::restart(Timestamp now) {
    if (repeat_) {
        //如果重复，设置下一个触发定时器的时刻
        expiration_ = addTime(now, interval_);
    }
    else {
        //否则设置expiration_为invalid Timestamp
        expiration_ = Timestamp::invalid();
    }
}