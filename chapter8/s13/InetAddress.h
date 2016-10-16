//
// Created by fit on 16-10-8.
//

#ifndef S04_INETADDRESS_H
#define S04_INETADDRESS_H

#include "base/copyable.h"

#include <string>

#include <netinet/in.h>

namespace muduo {

    class InetAddress : public muduo::copyable {

    public:
        explicit InetAddress(uint16_t port);

        InetAddress(const std::string &ip, uint16_t port);

        InetAddress(const struct sockaddr_in &addr) :
                addr_(addr) {

        }

        std::string toHostPort() const;

        const struct sockaddr_in &getSockAddrInet() const {
            return addr_;
        }

        void setSockAddrInet(const struct sockaddr_in &addr) {
            addr_ = addr;
        }

    private:
        struct sockaddr_in addr_;
    };

}

#endif //S04_INETADDRESS_H
