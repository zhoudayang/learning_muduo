#include <muduo/base/Logging.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpServer.h>

#include <boost/shared_ptr.hpp>

#include<stdio.h>

using namespace muduo;
using namespace muduo::net;
//fixme: an bug! double free
//print buffer size
void onHighWaterMark(const TcpConnectionPtr &con, size_t len) {
    LOG_INFO << "HighWaterMark " << len;
}

const int kBufSize = 64 * 1024;
const char *g_file = NULL;
typedef boost::shared_ptr<FILE> FilePtr;

void onConnection(const TcpConnectionPtr &con) {
    LOG_INFO << "FileServer - sending file " << g_file << " to "
             << con->peerAddress().toIpPort();
    con->setHighWaterMarkCallback(onHighWaterMark, kBufSize + 1);
    FILE *fp = ::fopen(g_file, "rb");
    if (fp) {
        ///使用shared_ptr 的 custom delete 来减轻资源管理负担， 使得FILE *的生命周期和TcpConnection 一样长，简化代码实现
        FilePtr ctx(fp, ::fclose);
        con->setContext(ctx);
        char buf[kBufSize];
        size_t nread = ::fread(buf, 1, sizeof buf, fp);
        con->send(buf, static_cast<int> (nread));
    } else {
        con->shutdown();
        LOG_INFO << "FileServer - no such file";
    }
}

void onWriteComplete(const TcpConnectionPtr &con) {
    const FilePtr &fp = boost::any_cast<const FilePtr &>(con->getContext());
    char buf[kBufSize];
    size_t nread = ::fread(buf, 1, sizeof buf, get_pointer(fp));
    if (nread > 0) {
        con->send(buf, static_cast<int>(nread));
    }
    else {
        con->shutdown();
        LOG_INFO << "FileServer - done";
    }
}

int main() {
    LOG_INFO << "pid=" << getpid();
    g_file = "/home/zhouyang/file";
    EventLoop loop;
    InetAddress listenAddr(2016);
    TcpServer server(&loop, listenAddr, "FileServer");
    server.setConnectionCallback(onConnection);
    server.setWriteCompleteCallback(onWriteComplete);
    server.start();
    loop.loop();
}