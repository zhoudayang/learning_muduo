//
// Created by fit on 16-10-8.
//

#include "Sockets.h"

#include "InetAddress.h"
#include "SocketsOps.h"

#include <netinet/in.h>
#include <netinet/tcp.h>
#include <strings.h>

using namespace muduo;

Socket::~Socket() {
    sockets::close(sockfd_);
}

void Socket::bindAddress(const InetAddress &addr) {
    sockets::bindOrDie(sockfd_, addr.getSockAddrInet());
}

void Socket::listen() {
    sockets::listenOrDie(sockfd_);
}

int Socket::accpet(InetAddress *peeraddr) {
    struct sockaddr_in addr;
    bzero(&addr, sizeof addr);
    int connfd = sockets::accept(sockfd_, &addr);
    //set peer address info
    if (connfd >= 0)
    {
        peeraddr->setSockAddrInet(addr);
    }
    return connfd;
}

void Socket::setReuseAddr(bool on) {
    int optval = on ? 1 : 0;
    //设置为地址复用
    ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);
}
void Socket::shutdownWrite()
{
    sockets::shutdownWrite(sockfd_);
}
