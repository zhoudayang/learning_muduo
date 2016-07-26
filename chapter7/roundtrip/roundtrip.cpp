//
// Created by zhouyang on 16-7-27.
//

#include <muduo/base/Logging.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpClient.h>
#include <muduo/net/TcpServer.h>

#include <stdio.h>

using namespace muduo;
using namespace muduo::net;

const size_t frameLen = 2 * sizeof(int64_t);

void serverConnectionCallback(const TcpConnectionPtr &con) {
    LOG_TRACE << con->name() << " " << con->peerAddress().toIpPort()
              << " -> " << con->localAddress().toIpPort() << " is "
              << (con->connected() ? "up" : "down");
    if (con->connected()) {
        con->setTcpNoDelay(true);
    }
    else {

    }
}

void serverMessageCallback(const TcpConnectionPtr &con, Buffer *buf, Timestamp receiveTime) {
    int64_t message[2];
    while (buf->readableBytes() >= frameLen) {
        memcpy(message, buf->peek(), frameLen);
        buf->retrieve(frameLen);
        //设置接收时间
        message[1] = receiveTime.microSecondsSinceEpoch();
        //想客户端回发填充之后的消息
        con->send(message, sizeof(message));
    }
}

void runServer(uint16_t port) {
    EventLoop loop;
    TcpServer server(&loop, InetAddress(port), "ClockServer");
    server.setConnectionCallback(serverConnectionCallback);
    server.setMessageCallback(serverMessageCallback);
    server.start();
    loop.loop();
}

TcpConnectionPtr clientConnection;

void clientConnectionCallback(const TcpConnectionPtr &con) {
    LOG_TRACE << con->localAddress().toIpPort() << " -> "
              << con->peerAddress().toIpPort() << " is "
              << (con->connected() ? "up" : "down");
    if (con->connected()) {
        clientConnection = con;
        con->setTcpNoDelay(true);
    }
    else {
        clientConnection.reset();
    }
}

void clientMessageCallback(const TcpConnectionPtr &, Buffer *buffer, Timestamp receiveTime) {
    int64_t message[2] = {0, 0};
    while (buffer->readableBytes() >= frameLen) {
        memcpy(message, buffer->peek(), frameLen);
        buffer->retrieve(frameLen);
        int64_t send = message[0];
        int64_t their = message[1];
        int64_t back = receiveTime.microSecondsSinceEpoch();
        int64_t mine = (back + send) / 2;
        //RTT clock_offset
        LOG_INFO << "round trip " << back - send << " clock offset " << their - mine;
    }
}

void sendMyTime() {
    if (clientConnection) {
        int64_t message[2] = {0, 0};
        message[0] = Timestamp::now().microSecondsSinceEpoch();
        clientConnection->send(message, sizeof message);
    }
}

void runClient(const char *ip, uint16_t port) {
    EventLoop loop;
    TcpClient client(&loop, InetAddress(ip, port), "ClockClient");
    client.enableRetry();
    client.setConnectionCallback(clientConnectionCallback);
    client.setMessageCallback(clientMessageCallback);
    client.connect();
    //设置每隔0.2秒向服务器发送一次数据包
    loop.runEvery(0.2, sendMyTime);
    loop.loop();
}

int main(int argc, char *argv[]) {
    if (argc > 2) {
        uint16_t port = static_cast<int16_t > (atoi(argv[2]));
        if (strcmp(argv[1], "-s") == 0) {
            runServer(port);
        }
        else
            runClient(argv[1], port);
    }
    else
        printf("usage:\n%s -s port\n%s ip port\n", argv[0], argv[1]);
}