//
// Created by fit on 16-10-8.
//

#ifndef S04_SOCKETSOPS_H
#define S04_SOCKETSOPS_H

#include <arpa/inet.h>
#include <endian.h>

namespace muduo {
    namespace sockets {
        inline uint64_t hostToNetwork64(uint64_t host64) {
            return htobe64(host64);
        }

        inline uint32_t hostToNetwork32(uint32_t host32) {
            return htonl(host32);
        }

        inline uint16_t hostToNetwork16(uint16_t host16) {
            return htons(host16);
        }

        inline uint64_t networkToHost64(uint64_t net64) {
            return be64toh(net64);
        }

        inline uint32_t networkToHost32(uint32_t net32) {
            return ntohl(net32);
        }

        inline uint16_t networkToHost16(uint16_t net16) {
            return ntohs(net16);
        }

        //create nonblocking socket
        int createNoblockingOrDie();

        //bind socket to sockaddr_in
        void bindOrDie(int sockfd, const struct sockaddr_in &addr);

        //begin to listen
        void listenOrDie(int sockfd);

        //accept connection and return file descriptor
        int accept(int sockfd, struct sockaddr_in *addr);

        //close the connection of given socket file descriptor
        void close(int sockfd);

        //transform from sockaddr_in to string ip:port
        void toHostPort(char *buf, size_t size, const struct sockaddr_in &addr);

        //get sockaddr_in from ip string and port
        void fromHostPort(const char *ip, uint16_t port, struct sockaddr_in *addr);

        struct sockaddr_in getLocalAddr(int sockfd);

        //get socket error
        int getSocketError(int sockfd);

        ssize_t readv(int sockfd, const struct iovec *iov, int iovcnt);

    }
}

#endif //S04_SOCKETSOPS_H
