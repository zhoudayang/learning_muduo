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

        // set read callback function
        void setReadCallback(const EventCallBack &cb) {
            readCallback_ = cb;
        }

        //set write callback function
        void setWriteCallback(const EventCallBack &cb) {
            writeCallback_ = cb;
        }


        //set erro callback function
        void setErrorCallback(const EventCallBack &cb) {
            errorCallback_ = cb;
        }

        //return 文件描述符
        int fd() const {
            return fd_;
        }

        //　return 关注的事件
        int events() const {
            return events_;
        }

        //　set revents
        void set_revents(int revt) {
            revents_ = revt;
        }

        //没有关注任何事件？
        bool isNoneEvent() const {
            return events_ == kNoneEvent;
        }
        //关注读事件
        void enableReading() {
            events_ |= kReadEvent;
            update();
        }


        // return index
        int index() {
            return index_;
        }

        //set index
        void set_index(int idx) {
            index_ = idx;
        }

        // return owner EventLoop
        EventLoop * ownerLoop(){
            return loop_;
        }
    private:
        void update();

        static const int kNoneEvent;
        static const int kReadEvent;
        static const int kWriteEvent;

        EventLoop *loop_;
        //file descriptor to poll
        const int fd_;
        //types of events poller cares about
        int events_;
        //types of events that actually occurred
        int revents_;
        //used by Poller,the index of pollfds_ in Poller
        int index_;

        //read callback function
        EventCallBack readCallback_;
        //write callback function
        EventCallBack writeCallback_;
        //error callback function
        EventCallBack errorCallback_;
    };

}


#endif //CHANNEL_CHANNEL_H
