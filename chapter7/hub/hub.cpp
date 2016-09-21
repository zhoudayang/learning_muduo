#include "codec.h"

#include <muduo/base/Logging.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpServer.h>

#include <boost/bind.hpp>

#include <map>
#include <set>
#include <stdio.h>

using namespace muduo;
using namespace muduo::net;
//这里相当于集线器
namespace pubsub
{

    typedef std::set<string> ConnectionSubscription;

    class Topic : public muduo::copyable
    {
    public:
        //constructor
        Topic(const string& topic)
                : topic_(topic)
        {
        }

        // add conn that cares for this topic into audiences
        void add(const TcpConnectionPtr& conn)
        {
            audiences_.insert(conn);
            if (lastPubTime_.valid())
            {
                //向订阅的连接发送 content
                conn->send(makeMessage());
            }
        }

        //remove connection from audiences
        void remove(const TcpConnectionPtr& conn)
        {
            audiences_.erase(conn);
        }

        //publish content to all conn in audiences
        void publish(const string& content, Timestamp time)
        {
            content_ = content;
            lastPubTime_ = time;
            string message = makeMessage();
            for (std::set<TcpConnectionPtr>::iterator it = audiences_.begin();
                 it != audiences_.end();
                 ++it)
            {
                (*it)->send(message);
            }
        }

    private:

        //发布广播消息的函数
        string makeMessage()
        {
            return "pub " + topic_ + "\r\n" + content_ + "\r\n";
        }

        string topic_;
        string content_;
        Timestamp lastPubTime_;
        std::set<TcpConnectionPtr> audiences_;
    };

    class PubSubServer : boost::noncopyable
    {
    public:
        PubSubServer(muduo::net::EventLoop* loop,
                     const muduo::net::InetAddress& listenAddr)
                : loop_(loop),
                  server_(loop, listenAddr, "PubSubServer")
        {
            //set connection callback function
            server_.setConnectionCallback(
                    boost::bind(&PubSubServer::onConnection, this, _1));
            //set message callback function
            server_.setMessageCallback(
                    boost::bind(&PubSubServer::onMessage, this, _1, _2, _3));
            //run timePublish function every 1.0 second
            loop_->runEvery(1.0, boost::bind(&PubSubServer::timePublish, this));
        }

        void start()
        {
            server_.start();
        }

    private:
        void onConnection(const TcpConnectionPtr& conn)
        {
            //ConnectionSubscription 记录该连接订阅的所有主题
            if (conn->connected())
            {
                conn->setContext(ConnectionSubscription());
            }
            else
            {
                //断开连接，取消其之间订阅的所有主题
                const ConnectionSubscription& connSub
                        = boost::any_cast<const ConnectionSubscription&>(conn->getContext());
                // subtle: doUnsubscribe will erase *it, so increase before calling.
                for (ConnectionSubscription::const_iterator it = connSub.begin();
                     it != connSub.end();)
                {
                    doUnsubscribe(conn, *it++);
                }
            }
        }

        void onMessage(const TcpConnectionPtr& conn,
                       Buffer* buf,
                       Timestamp receiveTime)
        {
            ParseResult result = kSuccess;
            while (result == kSuccess)
            {
                string cmd;
                string topic;
                string content;
                result = parseMessage(buf, &cmd, &topic, &content);
                if (result == kSuccess)
                {
                    if (cmd == "pub")
                        //publish information
                    {
                        doPublish(conn->name(), topic, content, receiveTime);
                    }
                    else if (cmd == "sub")
                    {
                        //subscribe topic
                        LOG_INFO << conn->name() << " subscribes " << topic;
                        doSubscribe(conn, topic);
                    }
                    else if (cmd == "unsub")
                    {
                        //un subscribe topic
                        doUnsubscribe(conn, topic);
                    }
                    else
                    {
                        //other command, shutdown the connection
                        conn->shutdown();
                        result = kError;
                    }
                }
                else if (result == kError)
                {
                    //error, shutdown the connection
                    conn->shutdown();
                }
            }
        }

        //定时播报现在时间
        void timePublish()
        {
            Timestamp now = Timestamp::now();
            doPublish("internal", "utc_time", now.toFormattedString(), now);
        }

        void doSubscribe(const TcpConnectionPtr& conn,
                         const string& topic)
        {
            //获取Context,　可以修改的那种 non const
            ConnectionSubscription* connSub
                    = boost::any_cast<ConnectionSubscription>(conn->getMutableContext());
            //该连接订阅topic 消息
            connSub->insert(topic);
            //订阅topic 的连接之中需要加上conn
            getTopic(topic).add(conn);
        }

        void doUnsubscribe(const TcpConnectionPtr& conn,
                           const string& topic)
        {
            LOG_INFO << conn->name() << " unsubscribes " << topic;
            //从记录订阅topic的set之中删除conn
            getTopic(topic).remove(conn);
            // topic could be the one to be destroyed, so don't use it after erasing.
            ConnectionSubscription* connSub
                    = boost::any_cast<ConnectionSubscription>(conn->getMutableContext());
            //该连接取消订阅topic
            connSub->erase(topic);
        }

        void doPublish(const string& source,
                       const string& topic,
                       const string& content,
                       Timestamp time)
        {
            //向每一个订阅topic的连接广播消息
            getTopic(topic).publish(content, time);
        }

        //Topic 记录了所有订阅topic的连接
        Topic& getTopic(const string& topic)
        {
            std::map<string, Topic>::iterator it = topics_.find(topic);
            if (it == topics_.end())
            {
                it = topics_.insert(make_pair(topic, Topic(topic))).first;
            }
            return it->second;
        }

        EventLoop* loop_;
        TcpServer server_;
        //记录了string 和　Topic之间的对应关系
        std::map<string, Topic> topics_;
    };

}

int main(int argc, char* argv[])
{
    if (argc > 1)
    {
        uint16_t port = static_cast<uint16_t>(atoi(argv[1]));
        EventLoop loop;
        pubsub::PubSubServer server(&loop, InetAddress(port));
        server.start();
        loop.loop();
    }
    else
    {
        printf("Usage: %s pubsub_port [inspect_port]\n", argv[0]);
    }
}
