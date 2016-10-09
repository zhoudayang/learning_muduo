//
// Created by fit on 16-10-9.
//

#ifndef S05_TCPCONNECTION_H
#define S05_TCPCONNECTION_H

#include "Callback.h"
#include "InetAddress.h"

#include <boost/any.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

namespace muduo{
    class Channel;
    class EventLoop;
    class Socket;

    class TcpConnection:boost::noncopyable,public boost::enable_shared_from_this{
    public:
    private:
        enum StateE{kConnecting,kConnected};
        void setState(StateE s){}

        void handleRead();

        EventLoop * loop_;
        std::string name_;
        StateE state_;

        boost::scoped_ptr<Socket> socket_;
        boost::scoped_ptr<Channel> channel_;
        InetAddress localAddr_;
        InetAddress peerAddr_;
    };
}


#endif //S05_TCPCONNECTION_H
