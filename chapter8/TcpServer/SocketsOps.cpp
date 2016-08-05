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
#include <cstdlib>

using namespace muduo;

//only use for upcast
template<typename To, typename From>
inline To implicit_cast(const From &f) {
    return f;
};
namespace {
    typedef struct sockaddr SA;

    //convert from sockaddr_in to sockaddr
    const SA *sockaddr_cast(const struct sockaddr_in *addr) {
        return static_cast<const SA *>(implicit_cast<const void *>(addr));
    }

    //convert from sockaddr_in to sockaddr
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

//create no blocking socket
int sockets::createNonBlockingOrDie() {
    int sockfd = ::socket(AF_INET,
                          SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC,
                          IPPROTO_TCP);
    if (sockfd < 0) {
        printf("sockets::createNonblockingOrDie\n");
    }
    return sockfd;
}

void sockets::bindOrDie(int sockfd, const struct sockaddr_in &addr) {
    int ret = ::bind(sockfd, sockaddr_cast(&addr), sizeof addr);
    if (ret < 0) {
        printf("sockets::bindOrDie\n", ret);
        abort();
    }
}

void sockets::listenOrDie(int sockfd) {
    int ret = ::listen(sockfd, SOMAXCONN);
    if (ret < 0) {
        printf("sockets::listenOrDie \n");
        abort();
    }
}

int sockets::accept(int sockfd, struct sockaddr_in *addr) {
    socklen_t addrlen = sizeof *addr;
    //accept connection
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
    //将整数uint32位网络地址转换为点分十进制形式的字符串
    /* Convert a Internet address in binary network format for interface
   type AF in buffer starting at CP to presentation form and place
   result in buffer of length LEN astarting at BUF.  */
    ::inet_ntop(AF_INET, &addr.sin_addr, host, sizeof host);
    uint16_t port = sockets::networkToHost16(addr.sin_port);
    snprintf(buf, size, "%d:%u", host, port);
}

void sockets::fromHostPort(const char *ip, uint16_t port, struct sockaddr_in *addr) {
    addr->sin_family = AF_INET;
    addr->sin_port = hostToNetwork16(port);
    /* Convert from presentation format of an Internet number in buffer
   starting at CP to the binary network format and store result for
   interface type AF in buffer starting at BUF.  */
    if (::inet_pton(AF_INET, ip, &addr->sin_addr) <= 0) {
        printf("sockets::fromHostPort \n");
        abort();
    }
}
struct sockaddr_in sockets::getLocalAddr(int sockfd){
    struct sockaddr_in local_addr;
    bzero(&local_addr,sizeof local_addr);
    socklen_t addrlen = sizeof(local_addr);
    if(::getsockname(sockfd,sockaddr_cast(&local_addr),&addrlen)<0){
        printf("sockets::getLocalAddr\n");
    }
    return local_addr;
}
