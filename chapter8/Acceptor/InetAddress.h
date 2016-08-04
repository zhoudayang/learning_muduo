//
// Created by zhouyang on 16-8-4.
//

#ifndef INETADDRESS_H
#define INETADDRESS_H

#include <string>
#include <netinet/in.h>
#include <boost/noncopyable.hpp>

namespace muduo {
    class InetAddress : boost::noncopyable {
    public:
        //constructs an endpoint with given port number
        //mostly used in TcpServer listening
        explicit InetAddress(uint16_t port);

        //constructs an endpoint with give ip and port

        InetAddress(const std::string &ip, uint16_t port);

        InetAddress(const struct sockaddr_in &addr) : addr_(addr) { }

        std::string toHostPort() const;

        const struct sockaddr_in &getSockAddrInet() const {
            return addr_;
        }

        void setSocketAddrInet(const struct sockaddr_in &addr) {
            addr_ = addr;
        }

    private:
        struct sockaddr_in addr_;

    };


}


#endif //INETADDRESS_H
