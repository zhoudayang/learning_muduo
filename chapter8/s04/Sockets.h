//
// Created by fit on 16-10-8.
//

#ifndef S04_SOCKETS_H
#define S04_SOCKETS_H

#include <boost/noncopyable.hpp>

namespace muduo {
    class InetAddress;

    class Socket : boost::noncopyable {
    public:
        explicit Socket(int sockfd)
                :sockfd_(sockfd) { }
        ~Socket();
        int fd() const
        {
            return sockfd_;
        }

        void bindAddress(const InetAddress& localaddr);

        void listen();

        int accpet(InetAddress* peeraddr);

        void setReuseAddr(bool on);

    private:
        const int sockfd_;
    };

}

#endif //S04_SOCKETS_H
