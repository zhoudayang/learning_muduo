//
// Created by zhouyang on 16/8/1.
//

#ifndef CHANNEL_H
#define CHANNEL_H

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include "Callbacks.h"

namespace muduo {
    class EventLoop;

    //A selection io channle
    //this class doesn't own the file descriptor
    //the file descriptor could be a socket
    //an eventfd , a timerfd or signalfd
    class Channel : boost::noncopyable {
    public:
        typedef boost::function<void()> EventCallback;

        Channel(EventLoop *loop, int fd);

        //析构函数
        ~Channel();

        void handleEvent();

        //set read callback
        void setReadCallback(const EventCallback &cb) {
            readCallback_ = cb;
        }

        //set write call back
        void setWriteCallback(const EventCallback &cb) {
            writeCallback_ = cb;
        }

        //void setCloseCallback
        void setCloseCallback(const EventCallback &cb) {
            closeCallback_ = cb;
        }


        //set error callback
        void setErrorCallback(const EventCallback &cb) {
            errorCallback_ = cb;
        }

        //return file description
        int fd() {
            return fd_;
        }

        //return events that cares about
        int events() const {
            return events_;
        }

        //set revents_
        void set_revents(int revt) {
            revents_ = revt;
        }

        // if it is noneEvent
        bool isNoneEvent() const {
            return events_ == kNoneEvent;
        }

        //now cares about reading event
        void enableReading() {
            events_ |= kReadEvent;
            //关注的事件有变，需要更新Poller中的pollfds_
            update();
        }

        //now cares about writing event
        void enableWriting() {
            events_ |= kWriteEvent;
            //关注的事件有变，需要更新Poller中的pollfds_
            update();
        }

        //does not care writing event
        void disableWriting() {
            events_ &= ~kWriteEvent;
            update();
        }

        //care nothing
        void disableAll() {
            events_ = kNoneEvent;
            update();
        }


        //return index
        int index() {
            return index_;
        }

        //set index
        void set_index(int idx) {
            index_ = idx;
        }

        //return owner EventLoop
        EventLoop *ownerLoop() {
            return loop_;
        }

    private:
        void update();

        static const int kNoneEvent;
        static const int kWriteEvent;
        static const int kReadEvent;

        //EventLoop
        EventLoop *loop_;
        //file description to pull
        const int fd_;
        //types of events that cares about
        int events_;
        //types of events that actually occured
        int revents_;
        //used by Poller,the index of pollfds_ in Poller
        int index_;
        //if is now handling the occured event
        bool eventHandling_;
        //read callback function
        EventCallback readCallback_;
        //write callback function
        EventCallback writeCallback_;
        //error callback function
        EventCallback errorCallback_;
        //close callback function
        EventCallback closeCallback_;

    };

}


#endif //TIMEQUEUE_CHANNEL_H
