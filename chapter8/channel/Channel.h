//
// Created by fit on 16-7-29.
//

#ifndef CHANNEL_CHANNEL_H
#define CHANNEL_CHANNEL_H

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>

namespace muduo {
    class EventLoop;

    class Channel : boost::noncopyable {
    public:
        typedef boost::function<void()> EventCallBack;

        Channel(EventLoop *loop, int fd);

        void handleEvent();

        void setReadCallback(const EventCallBack &cb) {
            readCallback_ = cb;
        }

        void setWriteCallback(const EventCallBack &cb) {
            writeCallback_ = cb;
        }

        void setErrorCallback(const EventCallBack &cb) {
            errorCallback_ = cb;
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

        int index() {
            return index_;
        }

        void set_index(int idx) {
            index_ = idx;
        }

        EventLoop * ownerLoop(){
            return loop_;
        }
    private:
        void update();

        static const int kNoneEvent;
        static const int kReadEvent;
        static const int kWriteEvent;

        EventLoop *loop_;
        const int fd_;
        int events_;
        int revents_;
        int index_;

        EventCallBack readCallback_;
        EventCallBack writeCallback_;
        EventCallBack errorCallback_;
    };

}


#endif //CHANNEL_CHANNEL_H
