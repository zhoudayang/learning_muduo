//
// Created by zhouyang on 16-8-4.
//

#ifndef SOCKET_H
#define SOCKET_H

#include <boost/noncopyable.hpp>

namespace muduo {
    class InetAddress;

    //wrapper of socket file descriptor
    //it close the sockfd when destructs
    //it's thread safe, all operations are delagated to os
    class Socket : boost::noncopyable {
    public:
        explicit Socket(int sockfd) : sockfd_(sockfd_) { }

        ~Socket();

        int fd() const {
            return sockfd_;
        }

        void bindAddress(const InetAddress &localaddr);

        void listen();

        int accept(InetAddress *peeraddr);

        void setReuseAddr(bool on);

    private:
        const int sockfd_;
    };


}

#endif //SOCKET_H
