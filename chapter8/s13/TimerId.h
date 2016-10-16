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
        explicit TimerId(Timer *timer = NULL,int64_t seq = 0) :
                timer_(timer) ,
                sequence_(seq)
        {

        }
        //default copy-ctor, dftor and assignment are okay
    private:
        Timer *timer_;
        int64_t sequence_;

        friend class TimerQueue;
    };
}


#endif //S02_TIMERID_H
