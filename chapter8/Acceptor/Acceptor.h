//
// Created by zhouyang on 16-8-4.
//

#ifndef ACCEPTOR_H
#define ACCEPTOR_H

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>

#include "Channel.h"
#include "Socket.h"

namespace muduo {
    class EventLoop;

    class InetAddress;

    class Acceptor : boost::noncopyable {
    public:
        //connection callback function
        typedef boost::function<void(int sockfd, const InetAddress &)> NewConnectionCallback;

        Acceptor(EventLoop *loop, const InetAddress &listenAddr);

        void setNewConnectionCallback(const NewConnectionCallback &cb) {
            newConnectionCallback_ = cb;
        }

        bool listening() const {
            return listening_;
        }

        void listen();

    private:
        void handleRead();

        EventLoop *loop_;
        Socket acceptSocket_;
        Channel acceptChannel_;
        NewConnectionCallback newConnectionCallback_;
        bool listening_;

    };


}

#endif //ACCEPTOR_H