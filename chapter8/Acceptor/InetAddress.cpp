//
// Created by zhouyang on 16-8-4.
//

#include "InetAddress.h"
#include "SocketsOps.h"

#include <string.h>
#include <netinet/in.h>

#include <boost/static_assert.hpp>

//     /* Structure describing an Internet socket address.  */
//     struct sockaddr_in {
//         sa_family_t    sin_family; /* address family: AF_INET */
//         uint16_t       sin_port;   /* port in network byte order */
//         struct in_addr sin_addr;   /* internet address */
//     };

//     /* Internet address. */
//     typedef uint32_t in_addr_t;
//     struct in_addr {
//         in_addr_t       s_addr;     /* address in network byte order */
//     };


using namespace muduo;

//linux下的socket INADDR_ANY表示的是一个服务器上所有的网卡（服务器可能不止一个网卡）
//多个本地ip地址都进行绑定端口号，进行侦听。
static const in_addr_t kInaddrAny = INADDR_ANY;

BOOST_STATIC_ASSERT(sizeof(InetAddress) == sizeof(struct sockaddr_in));

//设置绑定ip地址为INADDR_ANY
InetAddress::InetAddress(uint16_t port) {
    bzero(&addr_, sizeof(addr_));
    addr_.sin_family = AF_INET;
    addr_.sin_addr.s_addr = sockets::hostToNetwork32(kInaddrAny);
    addr_.sin_port = sockets::hostToNetwork16(port);
}

InetAddress::InetAddress(const std::string &ip, uint16_t port) {
    bzero(&addr_, sizeof(addr_));
    sockets::fromHostPort(ip.c_str(), port, &addr_);
}

std::string InetAddress::toHostPort() const {
    char buf[32];
    sockets::toHostPort(buf, sizeof buf, addr_);
    return buf;
}