//
// Created by fit on 16-7-29.
//

#ifndef CHANNEL_EVENTLOOP_H
#define CHANNEL_EVENTLOOP_H

#include "Thread.h"
#include <boost/scoped_ptr.hpp>
#include <vector>

namespace muduo {
    class Channel;

    class Poller;

    class EventLoop {
    public:
        EventLoop();

        ~EventLoop();

        void loop();

        void quit();

        void updateChannel(Channel *channel);

        bool isInLoopThread() const {
            return threadId_ == CurrentThread::tid();
        }

        void assertInLoopThread() {
            if (!isInLoopThread()) {
                abortNotInLoopThread();
            }
        }

    private:
        void abortNotInLoopThread();

        //存放 Channel * 指针的 vector数组
        typedef std::vector<Channel *> ChannelList;

        bool looping_;
        bool quit_;
        // EventLoop 所在的线程的pit_d值
        const pid_t threadId_;
        // 使用scoped_ptr会在指针离开作用域的时候自动释放管理的对象
        boost::scoped_ptr<Poller> poller_;
        ChannelList activeChannels_;
    };


}


#endif //CHANNEL_EVENTLOOP_H
