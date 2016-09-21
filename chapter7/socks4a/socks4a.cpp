//
// Created by zhouyang on 16-9-21.
//

#include "tunnel.h"

#include <muduo/net/Endian.h>
#include <stdio.h>
#include <netdb.h>

using namespace muduo;
using namespace muduo::net;

EventLoop * g_eventLoop;
std::map<string,TunnelPtr> g_tunnels;

void onServerConnection(const TcpConnectionPtr &conn){
    LOG_DEBUG<<conn->name() << (conn->connected() ? "up":"down");
    if(conn->connected()){
        //不适用nagle 算法
        conn->setTcpNoDelay(true);
    }
    else{
        //if client close the connection, remove connection from g_tunnels
        std::map<string,TunnelPtr> ::iterator it = g_tunnels.find(conn->name());
        if(it!=g_tunnels.end()){
            it->second->disconnect();
            g_tunnels.erase(it);
        }
    }
}

void onServerMessage(const TcpConnectionPtr &conn,Buffer * buf,Timestamp){
    LOG_INFO<<conn->name() <<" "<< buf->readableBytes();
    if(g_tunnels.find(conn->name())==g_tunnels.end()){
        //如果数据长度大于128，断开连接
        if(buf->readableBytes()>128){
            conn->shutdown();
        }

            /*
                +----+----+----+----+----+----+----+----+
                | VN | CD | DSTPORT |      DSTIP        |
                +----+----+----+----+----+----+----+----+
                   1    1      2              4
             */
            //可读的消息长度大于８　
        else if(buf->readableBytes()>8){
            const char * begin = buf->peek()+8;
            const char * end = buf->peek() + buf->readableBytes();
            const char * where = std::find(begin,end,'\0');
            if(where != end){
                //vn
                char ver = buf->peek()[0];
                //cd
                char cmd = buf->peek()[1];
                //destination port
                const void * port = buf->peek() +2;
                //destination ip
                const void * ip = buf->peek() +4;
                sockaddr_in addr;
                bzero(&addr,sizeof addr);
                addr.sin_family = AF_INET;
                //set port
                addr.sin_port = *static_cast<const in_port_t*>(port);
                //set ip
                addr.sin_addr.s_addr = *static_cast<const uint32_t*>(ip);
                //如果s_addr小于256,　那么表示是socks4a协议，需要socks服务器解析地址
                bool sock4a = sockets::networkToHost32(addr.sin_addr.s_addr) <256;
                bool okay =false;
                if(sock4a){
                    const char * endOfHostName = std::find(where+1,end,'\0');
                    if(endOfHostName!=end){
                        string hostname = where +1;
                        where = endOfHostName;
                        LOG_INFO<<"Sock4a host name "<<hostname;
                        InetAddress tmp;
                        //resolve ip address and set s_addr of addr
                        if(InetAddress::resolve(hostname,&tmp)){
                            addr.sin_addr.s_addr = tmp.ipNetEndian();
                            okay =true;
                        }
                    }
                    else{
                        //cannot find hostname, return immediately
                        return;
                    }
                }
                    //socks4 protocol
                else
                {
                    okay =true;
                }
                //forward server for next step
                InetAddress serverAddr(addr);
                if(ver ==4 and cmd ==1 and okay){
                    TunnelPtr tunnel (new Tunnel(g_eventLoop,serverAddr,conn));
                    tunnel->setup();
                    tunnel->connect();
                    //建立连接名称和tunnel之间的对应关系
                    g_tunnels[conn->name()] = tunnel;
                    //retrieve buf
                    buf->retrieveUntil(where+1);
                    //成功响应
                    char response[] = "\000\x5aUVWXYZ";
                    memcpy(response+2,&addr.sin_port,2);
                    memcpy(response+4,&addr.sin_addr.s_addr,4);
                    conn->send(response,8);
                }
                else{
                    //失败
                    char response[] = "\000\x5bUVWXYZ";
                    conn->send(response,8);
                    conn->shutdown();
                }
            }
        }
    }
        //是隧道返回的消息，直接将消息转交给client
    else if(!conn->getContext().empty()){
        //get TcpConnectionPtr
        const TcpConnectionPtr &clientConn = boost::any_cast<const TcpConnectionPtr &>(conn->getContext());
        //send buf to client
        //this buffer is input buffer, write it to output buffer to send to client 
        clientConn->send(buf);
    }
}
int main(int argc,char **argv){
    if(argc<2){
        fprintf(stderr,"Usage: %s <listen_port>\n",argv[0]);
    }
    else{
        LOG_INFO<<"pid = "<<getpid()<<", tid = "<<CurrentThread::tid();
        uint16_t  port = static_cast<uint16_t > (atoi(argv[1]));
        InetAddress listenAddr(port);
        EventLoop loop;
        g_eventLoop = &loop;
        TcpServer server ( &loop,listenAddr,"Socks4");
        server.setConnectionCallback(onServerConnection);
        //set message callback function
        server.setMessageCallback(onServerMessage);
        server.start();
        loop.loop();
    }
}