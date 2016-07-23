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
        con->send(filecontent);
        con->shutdown();
        LOG_INFO << "FileServer - done";
    }
}

int main() {
    LOG_INFO << " pid= " << getpid();
    g_file = "";
    EventLoop loop;
    InetAddress listenAddr(2016);
    TcpServer server(&loop, listenAddr, "FileServer");
    server.setConnectionCallback(onConnection);
    server.start();
    loop.loop();

}