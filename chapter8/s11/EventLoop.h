//
// Created by zhouyang on 16-9-27.
//

#ifndef S02_EVENTLOOP_H
#define S02_EVENTLOOP_H

#include "base/Thread.h"
#include "base/CurrentThread.h"
#include "Callback.h"
#include "TimerId.h"
#include "TimerQueue.h"

#include <boost/scoped_ptr.hpp>
#include <vector>

namespace muduo {
    class Channel;

    class Poller;

    class TimeQueue;

    class EventLoop : boost::noncopyable {
    public:
        typedef boost::function<void()> Functor;

        EventLoop();

        ~EventLoop();

        //main function run EventLoop
        void loop();

        //quit EventLoop
        void quit();

        Timestamp pollReturnTime() const {
            return pollReturnTime_;
        }

        TimerId runAt(const Timestamp &time, const TimerCallback &cb);

        TimerId runAfter(double delay, const TimerCallback &cb);

        TimerId runEvery(double interval, const TimerCallback &cb);

        void cancel(TimerId timerId);

        //update Channel in Poller
        void updateChannel(Channel *channel);

        void removeChannel(Channel *channel);

        //断言现在是否在创建EventLoop的线程中运行
        void assertInLoopThread() {
            if (!isInLoopThread())
            {
                abortNotInLoopThread();
            }
        }

        //断言现在在创建EventLoop的线程中工作
        bool isInLoopThread() const {
            return threadId_ == CurrentThread::tid();
        }

        void runInLoop(const Functor &cb);

        void queueInLoop(const Functor &cb);

        void wakeup();

    private:

        void handleRead();

        void doPendingFunctors();


        void abortNotInLoopThread();

        typedef std::vector<Channel *> ChannelList;

        bool looping_;
        bool quit_;
        const pid_t threadId_;
        Timestamp pollReturnTime_;
        boost::scoped_ptr<Poller> poller_;
        //add element
        boost::scoped_ptr<TimerQueue> timerQueue_;
        ChannelList activeChannels_;
        //是否正在call pending Functors ?
        bool callingPendingFunctors_;
        //event fd
        int wakeupFd_;
        //wake up channel
        boost::scoped_ptr<Channel> wakeupChannel_;
        //lock to defend pendingFunctors_ under multi threads
        MutexLock mutex_;
        std::vector<Functor> pendingFunctors_;


    };
}
#endif //S02_EVENTLOOP_H
