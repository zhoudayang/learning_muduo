//
// Created by zhouyang on 16-8-4.
//

#include "SocketsOps.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

using namespace muduo;

template<typename To, typename From>
inline To implicit_cast(const From &f) {
    return f;
};
namespace {
    typedef struct sockaddr SA;

    const SA *sockaddr_cast(const struct sockaddr_in *addr) {
        return static_cast<const SA *>(implicit_cast<const void *>(addr));
    }

    SA *sockaddr_cast(struct sockaddr_in *addr) {
        return static_cast<SA *>(implicit_cast<void *>(addr));
    }

    void setNoBlockAndCloseOnExec(int sockfd) {
        //non-block
        int flags = ::fcntl(sockfd, F_GETFL, 0);
        flags |= O_NONBLOCK;
        int ret = ::fcntl(sockfd, F_SETFL, flags);

        flags = ::fcntl(sockfd, F_GETFD, 0);
        flags |= FD_CLOEXEC;
        ret = ::fcntl(sockfd, F_SETFD, flags);
    }

}

int sockets::createNonBlockingOrDie() {

    int sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    if (sockfd < 0) {
        printf("sockets::createNonblockingOrDie \n");
    }
    return sockfd;
}

void sockets::bindOrDie(int sockfd, const struct sockaddr_in &addr) {
    int ret = ::bind(sockfd, sockaddr_cast(&addr), sizeof addr);
    if (ret < 0) {

        printf("sockets::bindOrDie \n");
    }
}

void sockets::listenOrDie(int sockfd) {
    int ret = ::listen(sockfd, SOMAXCONN);
    if (ret < 0) {
        printf("sockets::listenOrDie \n");
    }
}

int sockets::accept(int sockfd, struct sockaddr_in *addr) {
    socklen_t addrlen = sizeof *addr;
    int connfd = ::accept4(sockfd, sockaddr_cast(addr), &addrlen, SOCK_NONBLOCK | SOCK_CLOEXEC);
    if (connfd < 0) {
        int savedErrno = errno;
        printf("Socket::accept \n");
        switch (savedErrno) {
            case EAGAIN:
            case ECONNABORTED:
            case EINTR:
            case EPERM:
            case EMFILE:
                //todo what is errno ?
                errno = savedErrno;
                break;
            case EBADF:
            case EFAULT:
            case EINVAL:
            case ENFILE:
            case ENOBUFS:
            case ENOMEM:
            case ENOTSOCK:
            case EOPNOTSUPP:
                printf("unexpected error of ::accept %d \n", savedErrno);
                break;
            default:
                printf("unknown error of ::accept %d", savedErrno);
                break;
        }
    }
    return connfd;
}

void sockets::close(int sockfd) {
    if (::close(sockfd) < 0) {
        printf("sockets::close \n");
    }
}


void sockets::toHostPort(char *buf, size_t size, const struct sockaddr_in &addr) {
    char host[INET_ADDRSTRLEN] = "INVALID";
    ::inet_ntop(AF_INET, &addr.sin_addr, host, sizeof host);
    uint16_t port = sockets::networkToHost16(addr.sin_port);
    snprintf(buf, size, "%d:%u", host, port);
}

void sockets::fromHostPort(const char *ip, uint16_t port, struct sockaddr_in *addr) {
    addr->sin_family = AF_INET;
    addr->sin_port = hostToNetwork16(port);
    if (::inet_pton(AF_INET, ip, &addr->sin_addr) <= 0)
        printf("sockets::fromHostPort \n");
}

