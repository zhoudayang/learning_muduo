//
// Created by zhouyang on 16-9-27.
//

#ifndef S02_POLLER_H
#define S02_POLLER_H

#include <map>
#include <vector>

#include "base/Timestamp.h"
#include "EventLoop.h"


/*
    struct pollfd
    {
        int fd;			        //File descriptor to poll.  
        short int events;		//Types of events poller cares about.  
        short int revents;		//Types of events that actually occurred. 
    };
 
*/

struct pollfd;

namespace muduo{
    class Channel;
    class Poller:boost::noncopyable{
    public:
        typedef std::vector<Channel *> ChannelList;
        Poller(EventLoop * loop);
        ~Poller();
        //polls the I/O events
        //must be called in the loop thread
        Timestamp poll(int timeoutMs,ChannelList * activeChannels);
        //changs the interested I/O events
        //must be called in the loop thread
        void updateChannel(Channel * channel);
        void assertInLoopThread(){
            ownerLoop_ -> assertInLoopThread();
        }
    private:
        void fillActiveChannels(int numEvents,ChannelList * activeChannels) const;
        
        //vector that store struct  pollfd 
        typedef std::vector<struct pollfd> PollFdList;
        //key index in ChannelList, value Channel pointer
        typedef std::map<int,Channel *> ChannelMap;
        
        EventLoop * ownerLoop_;
        PollFdList pollfds_;
        ChannelMap channels_;
        
    };
}


#endif //S02_POLLER_H
