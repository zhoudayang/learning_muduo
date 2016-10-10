//
// Created by zhouyang on 16-9-27.
//

#ifndef S02_CHANNEL_H
#define S02_CHANNEL_H

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>

#include "base/Timestamp.h"

namespace muduo {

    class EventLoop;

    /// A selectable I/O channel

    //this class doesn't own the file descriptor
    //the file descriptor could be a socket
    //an eventfd,a timefd, or signalfd

    class Channel : boost::noncopyable {
    public:
        typedef boost::function<void()> EventCallback;
        typedef boost::function<void(Timestamp)> ReadEventCallback;
        Channel(EventLoop *loop, int fd);

        ~Channel();

        //handle event that occurred
        //add receive time
        void handleEvent(Timestamp receiveTime);

        void setReadCallback(const ReadEventCallback &cb) {
            readCallback_ = cb;
        }

        void setWriteCallback(const EventCallback &cb) {
            writeCallback_ = cb;
        }

        void setErrorCallback(const EventCallback &cb) {
            errorCallback_ = cb;
        }

        //set close callback function
        void setCloseCallback(const EventCallback &cb) {
            closeCallback_ = cb;
        }

        int fd() const {
            return fd_;
        }

        int events() const {
            return events_;
        }

        void set_revents(int revt) {
            revents_ = revt;
        }

        bool isNoneEvent() const {
            return events_ == kNoneEvent;
        }

        void enableReading() {
            events_ |= kReadEvent;
            update();
        }

        void enableWriting() {
            events_ |= kWriteEvent;
            update();
        }

        //now cares about nothing
        void disableAll() {
            events_ = kNoneEvent;
            update();
        }


        int index() {
            return index_;
        }

        void set_index(int idx) {
            index_ = idx;
        }

        EventLoop *ownerLoop() {
            return loop_;
        }

    private:
        void update();

        static const int kNoneEvent;
        static const int kReadEvent;
        static const int kWriteEvent;

        //pointer of EventLoop
        EventLoop *loop_;
        const int fd_;
        //events that cares about
        int events_;
        //events return that occurred
        int revents_;
        //index in poller
        int index_;

        //flag show if now handle the event ?
        bool eventHandling_;


        //read callback function
        ReadEventCallback readCallback_;
        //write callback function
        EventCallback writeCallback_;
        //error callback function
        EventCallback errorCallback_;
        //close callback function
        EventCallback closeCallback_;

    };

}


#endif //S02_CHANNEL_H
