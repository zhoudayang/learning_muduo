//
// Created by fit on 16-10-16.
//

#ifndef S13_EPOLLER_H
#define S13_EPOLLER_H

#include <map>
#include <vector>

#include "base/Timestamp.h"
#include "EventLoop.h"

struct epoll_event;

namespace muduo {

    class Channel;

    class Epoller : boost::noncopyable {
    public:
        typedef std::vector<Channel*> ChannelList;

        Epoller(EventLoop* loop);

        ~Epoller();

        Timestamp poll(int timeOutMs, ChannelList* activeChannels);

        void updateChannel(Channel* channel);

        void removeChannel(Channel* channel);

        void assertInLoopThread()
        {
            ownerLoop_->assertInLoopThread();
        }

    private:
        static const int kInitEventListSize = 16;

        void fillActiveChannels(int numEvents, ChannelList* activeChannels) const;

        void update(int operation, Channel* channel);

        typedef std::vector<struct epoll_event> EventList;
        //key, value fd, Channel *
        typedef std::map<int, Channel*> ChannelMap;

        EventLoop* ownerLoop_;
        int epollfd_;
        EventList events_;
        ChannelMap channels_;
    };

}

#endif //S13_EPOLLER_H
