#include <muduo/base/Logging.h>
#include <muduo/net/Endian.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>
#include <muduo/net/TcpClient.h>
#include <boost/bind.hpp>

using namespace muduo;
using namespace muduo::net;


class TimeClient : boost::noncopyable {
public:
    TimeClient(EventLoop *loop, const InetAddress &serverAddr);

    void connect();

private:
    muduo::net::EventLoop *loop_;
    muduo::net::TcpClient client_;

    void onConnection(const TcpConnectionPtr &con);

    void onMessage(const TcpConnectionPtr &con, Buffer *buf, Timestamp receiveTime);
};



TimeClient::TimeClient(EventLoop *loop, const InetAddress &serverAddr)
        : loop_(loop), client_(loop, serverAddr, "TimeClient") {
    //bind connection call back function
    client_.setConnectionCallback(
            boost::bind(&TimeClient::onConnection, this, _1)
    );
    //bind message callback function
    client_.setMessageCallback(
            boost::bind(&TimeClient::onMessage, this, _1, _2, _3)
    );
}

//begin to connect to time server
void TimeClient::connect() {
    client_.connect();
}

void TimeClient::onConnection(const TcpConnectionPtr &con) {
    LOG_INFO << con->localAddress().toIpPort()
             << " -> "
             << con->peerAddress().toIpPort()
             << " is "
             << (con->connected() ? "up" : "down");
    //连接断开，退出事件循环，程序终止
    if (!con->connected()) {
        loop_->quit();
    }
}

void TimeClient::onMessage(const TcpConnectionPtr &con, Buffer *buf, Timestamp receiveTime) {
    //可读的数据段大小大于int32_t
    if (buf->readableBytes() >= sizeof(int32_t)) {
        const void *data = buf->peek();
        int32_t be32 = *static_cast<const int32_t *> (data);
        buf->retrieve(sizeof(int32_t));
        //convert from network endian to host endian
        time_t time = sockets::networkToHost32(be32);
        //convert from int32_t to muduo::Timestamp
        Timestamp ts(implicit_cast<uint64_t>(time) * Timestamp::kMicroSecondsPerSecond);
        LOG_INFO << "Server time = "
                 << time << ", "
                 << ts.toFormattedString();
    } else {
        LOG_INFO << con->name()
                 << " no enough data "
                 << buf->readableBytes()
                 << " at "
                 << receiveTime.toFormattedString();
    }
}

int main() {
    LOG_INFO << "pid = " << getpid();
    EventLoop loop;
    //server address
    InetAddress serverAddr("127.0.0.1", 2016);
    //new time client instance
    TimeClient timeClient(&loop, serverAddr);
    timeClient.connect();
    loop.loop();
}