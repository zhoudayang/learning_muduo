//
// Created by zhouyang on 16/8/1.
//

#ifndef CHANNEL_H
#define CHANNEL_H

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include "EventLoop.h"

namespace muduo {
    class Channel {
    public:
        typedef boost::function<void()> EventCallback;

        Channel(EventLoop *loop, int fd);

        void handleEvent();

        void setReadCallback(const EventCallback &cb) {
            readCallback_ = cb;
        }

        void setErrorCallback(const EventCallback &cb) {
            errorCallback_ = cb;
        }

        void setWriteCallback(const EventCallback &cb) {
            errorCallback_ = cb;
        }

        int fd() const {
            return fd_;
        }

        int events() const {
            return events_;
        }

        void set_events(int recv) {
            revents_ = recv;
        }

        bool isNonoEvent() const {
            return events_ == kNoneEvent;
        }

        void eableReading() {
            events_ |= kReadEvent;
            update();
        }

        int index() const {
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
        static const int kWriteevent;

        EventLoop *loop_;
        const int fd_;
        int events_;
        int revents_;
        int index_;
        EventCallback readCallback_;
        EventCallback writeCallback_;
        EventCallback errorCallback_;

    };
}


#endif //TIMEQUEUE_CHANNEL_H
