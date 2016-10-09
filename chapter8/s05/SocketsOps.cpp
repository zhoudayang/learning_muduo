//
// Created by fit on 16-10-8.
//

#include "SocketsOps.h"

#include "base/Logging.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <strings.h>
#include <sys/socket.h>
#include <unistd.h>

using namespace muduo;

namespace {
    typedef struct sockaddr SA;

    const SA *sockaddr_cast(const struct sockaddr_in *addr) {
        return static_cast<const SA *>(implicit_cast<const void *>(addr));
    }

    SA *sockaddr_cast(struct sockaddr_in *addr) {
        return static_cast<SA *>(implicit_cast<void *>(addr));
    }

    void setNonBlockAndCloseOnExec(int sockfd) {

        int flags = ::fcntl(sockfd, F_GETFL, 0);
        flags |= O_NONBLOCK;
        int ret = ::fcntl(sockfd, F_SETFL, flags);
        flags = ::fcntl(sockfd, F_GETFD, 0);
        flags |= FD_CLOEXEC;
        ret = ::fcntl(sockfd, F_SETFD, flags);
    }

}

int sockets::createNoblockingOrDie() {
    int sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    if (sockfd < 0)
    {
        LOG_SYSFATAL << "sockets::createNonblockingOrDie";
    }
    return sockfd;
}

void sockets::bindOrDie(int sockfd, const struct sockaddr_in &addr) {
    int ret = ::bind(sockfd, sockaddr_cast(&addr), sizeof addr);
    if (ret < 0)
    {
        LOG_SYSFATAL << "sockets::bindOrDie";
    }
}

void sockets::listenOrDie(int sockfd) {
    //OMAXCONN 128
    int ret = ::listen(sockfd, SOMAXCONN);
    if (ret < 0)
    {
        LOG_SYSFATAL << "sockets::listenOrDie";
    }
}

int sockets::accept(int sockfd, struct sockaddr_in *addr) {
    socklen_t addrlen = sizeof *addr;
    //no block and close when exit
    int connfd = ::accept4(sockfd, sockaddr_cast(addr), &addrlen, SOCK_NONBLOCK | SOCK_CLOEXEC);
    if (connfd < 0)
    {
        int savedErrno = errno;
        LOG_SYSERR << "Socket::accept";
        switch (savedErrno)
        {
            case EAGAIN:
            case ECONNABORTED:
            case EINTR:
            case EPROTO: // ???
            case EPERM:
            case EMFILE: // per-process lmit of open file desctiptor ???
                // expected errors
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
                // unexpected errors
                LOG_FATAL << "unexpected error of ::accept " << savedErrno;
                break;
            default:
                LOG_FATAL << "unknown error of ::accept " << savedErrno;
                break;
        }
    }
    return connfd;
}

//close socket
void sockets::close(int sockfd) {
    if (::close(sockfd) < 0)
    {
        LOG_SYSERR << "sockets::close";
    }
}

// 将sockaddr_in 转换为　ip:port　格式的字符串
void sockets::toHostPort(char *buf, size_t size, const struct sockaddr_in &addr) {
    char host[INET_ADDRSTRLEN] = "INVALID";
    ::inet_ntop(AF_INET, &addr.sin_addr, host, sizeof host);
    uint16_t port = sockets::networkToHost16(addr.sin_port);
    snprintf(buf, size, "%s:%u", host, port);
}

//convert from ip string and port to sockaddr_in
void sockets::fromHostPort(const char *ip, uint16_t port, struct sockaddr_in *addr) {
    addr->sin_family = AF_INET;
    addr->sin_port = hostToNetwork16(port);
    if (::inet_pton(AF_INET, ip, &addr->sin_addr) <= 0)
    {
        LOG_SYSERR << "sockets::fromHostPort";
    }
}

struct sockaddr_in sockets::getLocalAddr(int sockfd) {
    struct sockaddr_in localAddr;
    bzero(&localAddr, sizeof localAddr);
    socklen_t addrlen = sizeof(localAddr);
    //获取socket file descriptor对应的sockaddr_in信息
    if (::getsockname(sockfd, sockaddr_cast(&localAddr), &addrlen) < 0)
    {
        LOG_SYSERR << "sockets::getLocalAddr";
    }
    return localAddr;
}
