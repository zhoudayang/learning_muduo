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
    //connection name peer address local address connection status
    LOG_TRACE << con->name() << " " << con->peerAddress().toIpPort()
              << " -> " << con->localAddress().toIpPort() << " is "
              << (con->connected() ? "up" : "down");
    if (con->connected()) {
        //避免使用nagle算法
        con->setTcpNoDelay(true);
    }
}

void serverMessageCallback(const TcpConnectionPtr &con, Buffer *buf, Timestamp receiveTime) {
    int64_t message[2];
    //当可读的消息长度大于或者等于数据帧的长度的时候
    while (buf->readableBytes() >= frameLen) {
        //将buf中的数据复制进入数组message 中
        memcpy(message, buf->peek(), frameLen);
        //设置readIndex
        buf->retrieve(frameLen);
        //设置接收时间
        message[1] = receiveTime.microSecondsSinceEpoch();
        //向客户端回发填充之后的消息
        con->send(message, sizeof(message));
    }
}

//function called when begin to run server
void runServer(uint16_t port) {
    EventLoop loop;
    TcpServer server(&loop, InetAddress(port), "ClockServer");
    server.setConnectionCallback(serverConnectionCallback);
    server.setMessageCallback(serverMessageCallback);
    server.start();
    loop.loop();
}

TcpConnectionPtr clientConnection;


//client connection callback function
void clientConnectionCallback(const TcpConnectionPtr &con) {
    LOG_TRACE << con->localAddress().toIpPort() << " -> "
              << con->peerAddress().toIpPort() << " is "
              << (con->connected() ? "up" : "down");
    if (con->connected()) {
        //store client connection
        clientConnection = con;
        con->setTcpNoDelay(true);
    }
    else {
        //if server close the connection, reset clientConnection ptr
        clientConnection.reset();
    }
}

void clientMessageCallback(const TcpConnectionPtr &, Buffer *buffer, Timestamp receiveTime) {
    int64_t message[2] = {0, 0};
    while (buffer->readableBytes() >= frameLen) {
        memcpy(message, buffer->peek(), frameLen);
        buffer->retrieve(frameLen);
        //发送时间
        int64_t send = message[0];
        //服务器时间
        int64_t their = message[1];
        //接收时间
        int64_t back = receiveTime.microSecondsSinceEpoch();
        //中间时刻
        int64_t mine = (back + send) / 2;
        //RTT clock_offset
        LOG_INFO << "round trip " << back - send << " clock offset " << their - mine;
    }
}

//向服务器发送自己的时间
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
        //总是将第二个参数转换为int16_t类型的port
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