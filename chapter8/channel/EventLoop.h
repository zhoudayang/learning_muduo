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

        typedef std::vector<Channel *> ChannelList;

        bool looping_;
        bool quit_;
        const pid_t threadId_;
        boost::scoped_ptr<Poller> poller_;
        ChannelList activeChannels;
    };


}


#endif //CHANNEL_EVENTLOOP_H
