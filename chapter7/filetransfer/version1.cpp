//版本一，在简历连接之后，将文件的全部内容读入一个string中，
// 一次调用TcpConnection::send() 进行发送。不用担心文件发送不完整，也不用担心
//send()之后立刻shutdown()会有什么问题
#include <muduo/base/Logging.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpServer.h>

#include <stdio.h>

using namespace muduo;
using namespace muduo::net;

const char *g_file = NULL;

//将整个文件内容读入到string 中
string readFile(const char *filename) {
    string content;
    FILE *fp = ::fopen(filename, "rb");
    if (fp) {
        const int kBufSize = 1024 * 2014;
        char iobuf[kBufSize];
        ::setbuffer(fp, iobuf, sizeof iobuf);
        char buf[kBufSize];
        size_t nread = 0;
        while ((nread = ::fread(buf, 1, sizeof buf, fp)) > 0) {
            content.append(buf, nread);
        }
        return content;
    }
};

void onHighWaterMark(const TcpConnectionPtr &con, size_t len) {

    LOG_INFO << "HighWaterMark " << len;
}

void onConnection(const TcpConnectionPtr &con) {
    LOG_INFO << "FileServer - "
             << con->peerAddress().toIpPort()
             << " -> "
             << con->localAddress().toIpPort()
             << (con->connected() ? "up" : "down");
    if (con->connected()) {
        LOG_INFO << "FileServer - Sending file " << g_file << " to "
                 << con->peerAddress().toIpPort();
        con->setHighWaterMarkCallback(onHighWaterMark, 64 * 1024);
        string filecontent = readFile(g_file);
        //直接将整个文件发送给对方
        con->send(filecontent);
        //发送完毕直接关闭连接
        con->shutdown();
        LOG_INFO << "FileServer - done";
    }
}

int main() {
    LOG_INFO << " pid= " << getpid();
    g_file = "/home/zhouyang/Downloads/file";
    EventLoop loop;
    InetAddress listenAddr(2016);
    TcpServer server(&loop, listenAddr, "FileServer");
    server.setConnectionCallback(onConnection);
    server.start();
    loop.loop();

}