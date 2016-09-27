//
// Created by zhouyang on 16-9-27.
//

#ifndef S02_EVENTLOOP_H
#define S02_EVENTLOOP_H

#include "base/Thread.h"
#include "base/CurrentThread.h"
#include <boost/scoped_ptr.hpp>
#include <vector>

namespace muduo{
    class Channel;
    class Poller;

    class EventLoop:boost::noncopyable{
    public:
        EventLoop();
        ~EventLoop();

        //main function run EventLoop
        void loop();

        //quit EventLoop
        void quit();

        //update Channel in Poller
        void updateChannel(Channel * channel);

        //断言现在是否在创建EventLoop的线程中运行
        void assertInLoopThread(){
            if(!isInLoopThread()){
                abortNotInLoopThread();
            }
        }

        //断言现在在创建EventLoop的线程中工作
        bool isInLoopThread() const{
            return threadId_ == CurrentThread::tid();
        }
    private:

        void abortNotInLoopThread();

        typedef std::vector<Channel *> ChannelList;

        bool looping_;
        bool quit_;
        const pid_t threadId_;
        boost::scoped_ptr<Poller> poller_;
        ChannelList activeChannels_;
    };
}
#endif //S02_EVENTLOOP_H
