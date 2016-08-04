//
// Created by zhouyang on 16-8-4.
//

#include "Socket.h"
#include "InetAddress.h"
#include "SocketsOps.h"

#include <netinet/in.h>
#include <netinet/tcp.h>
#include <string.h>

using namespace muduo;

Socket::~Socket() {
    sockets::close(sockfd_);
}

void Socket::listen() {
    sockets::listenOrDie(sockfd_);
}

int Socket::accept(InetAddress *peeraddr) {
    struct sockaddr_in addr;
    bzero(&addr, sizeof addr);
    int connfd = sockets::accept(sockfd_, &addr);
    if (connfd >= 0) {
        peeraddr->setSocketAddrInet(addr);
    }
    return connfd;
}

void Socket::setReuseAddr(bool on) {
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);
}
