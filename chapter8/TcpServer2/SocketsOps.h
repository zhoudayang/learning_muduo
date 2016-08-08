//
// Created by zhouyang on 16-8-4.
//

#ifndef SOCKETSOPS_H
#define SOCKETSOPS_H

#include <arpa/inet.h>
#include <endian.h>

namespace muduo {
    namespace sockets {
        //将 uint64_t 主机数转换为网络字节顺序的 uint64_t 数字
        inline uint64_t hostToNetwork64(uint64_t host64) {
            return htobe64(host64);
        }

        inline uint32_t hostToNetwork32(uint32_t host32) {
            return htonl(host32);
        }

        inline uint16_t hostToNetwork16(uint16_t host16) {
            return htons(host16);
        }

        //将uint64_t网络顺序的数转换为uint64_t主机数
        inline uint64_t networkToHost64(uint64_t net64) {
            return be64toh(net64);
        }

        inline uint32_t networkToHost32(uint32_t net32) {
            return ntohl(net32);
        }

        inline uint16_t networkToHost16(uint16_t net16) {
            return ntohs(net16);
        }

        //create no blocking socket
        int createNonBlockingOrDie();

        //bind socket to address
        void bindOrDie(int sockfd, const struct sockaddr_in &addr);

        //socket begin to listen
        void listenOrDie(int sockfd);

        //socket begin to accept
        //return connect fd
        int accept(int sockfd, struct sockaddr_in *addr);

        //close socket
        void close(int sockfd);

        //将addr转换为ip:port
        void toHostPort(char *buf, size_t size, const struct sockaddr_in &addr);

        //将ip,port 转换为struct sockaddr_in
        void fromHostPort(const char *ip, uint16_t port, struct sockaddr_in *addr);

        struct sockaddr_in getLocalAddr(int sockfd);

        int getSocketError(int sockfd);
    }


}

#endif
