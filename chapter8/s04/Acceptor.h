//
// Created by fit on 16-10-8.
//

#ifndef S04_ACCEPTOR_H
#define S04_ACCEPTOR_H

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>

#include "Channel.h"
#include "Sockets.h"

namespace muduo {

    class EventLoop;
    class InetAddress;

    class Acceptor : boost::noncopyable {
    public:
        typedef boost::function<void(int sockfd, const InetAddress&)> NewConnectionCallback;
        Acceptor(EventLoop* loop, const InetAddress& listenAddr);

        void setNewConnectionCallback(const NewConnectionCallback& cb)
        {
            newConnectionCallback_ = cb;
        }

        bool listening() const
        {
            return listening_;
        }

        void listen();
    private:

        void handleRead();
        EventLoop* loop_;
        Socket acceptSocket_;
        Channel acceptChannel_;
        NewConnectionCallback newConnectionCallback_;
        bool listening_;
    };
}

#endif //S04_ACCEPTOR_H
