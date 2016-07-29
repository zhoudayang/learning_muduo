//
// Created by fit on 16-7-28.
//
#ifndef EVENT_LOOP
#define EVENT_LOOP

#include "Thread.h"

namespace muduo {
    class EventLoop : boost::noncopyable {
    public:
        EventLoop();

        ~EventLoop();

        void loop();

        void assertInLoopThread() {
            if (!isInLoopThread()) {
                abortNotInloopThread();
            }
        }

        bool isInLoopThread() const {
            return threadId_ == CurrentThread::tid();
        }

    private:
        void abortNotInloopThread();

        bool looping_;
        const pid_t threadId_;
    };

}
#endif