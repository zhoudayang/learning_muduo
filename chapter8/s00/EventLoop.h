//
// Created by zhouyang on 16-9-27.
//

#ifndef EMPTY_EVENTLOOP_EVENTLOOP_H
#define EMPTY_EVENTLOOP_EVENTLOOP_H

#include "base/Thread.h"
#include "base/CurrentThread.h"

namespace muduo{

    class EventLoop:boost::noncopyable{
    public:
        EventLoop();
        ~EventLoop();

        void loop();

        void assertInLoopThread(){
            if(!isInLoopThread()){
                abortNotInLoopThread();
            }
        }
        bool isInLoopThread() const{
            //断言创建EventLoop的线程与运行loop的线程一致　
            return threadId_ == CurrentThread::tid();
        }

    private:

        void abortNotInLoopThread();

        bool looping_; /* */
        const pid_t threadId_;
    };


}


#endif //EMPTY_EVENTLOOP_EVENTLOOP_H
