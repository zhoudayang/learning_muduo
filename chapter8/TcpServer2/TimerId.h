//
// Created by zhouyang on 16/8/1.
//

#ifndef TIMERID_H
#define TIMERID_H

namespace muduo {
    class Timer;

    //an opaque identifier, for canceling timer
    //存放Timer对象的指针
    class TimerId {
    public:
        explicit TimerId(Timer *timer) : value_(timer) {}
        //default copy-ctor dtor and assignment are okay
    private:
        Timer *value_;

    };
}

#endif //TIMEQUEUE_TIMERID_H
