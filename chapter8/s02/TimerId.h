//
// Created by zhouyang on 16-9-28.
//

#ifndef S02_TIMERID_H
#define S02_TIMERID_H

#include "base/copyable.h"

namespace muduo {


    class Timer;
    // 为了标志Timer,　区分相同到期时间的不同Timer
    //an opaque identifier, for canceling Timer
    class TimerId : public muduo::copyable {
    public:
        explicit TimerId(Timer *timer) :
                value_(timer) {

        }
        //default copy-ctor, dftor and assignment are okay
    private:
        Timer *value_;
    };
}


#endif //S02_TIMERID_H
