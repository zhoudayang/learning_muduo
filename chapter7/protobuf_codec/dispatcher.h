//
// Created by zhouyang on 16-9-20.
//

#ifndef PROTOBUF_CODEC_DISPATCHER_H
#define PROTOBUF_CODEC_DISPATCHER_H

#include <muduo/net/Callbacks.h>
#include <google/protobuf/message.h>

#include <map>

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

#ifndef NDEBUG
#include <boost/static_assert.hpp>
#include <boost/type_traits/is_base_of.hpp>
#endif

//shared_ptr for protobuf message instance
typedef boost::shared_ptr <google::protobuf::Message> MessagePtr;

//base class
//抽象类　
class Callback:boost::noncopyable{
public:
    virtual ~Callback(){};
    virtual void OnMessage(const muduo::net::TcpConnectionPtr &,const MessagePtr & message,muduo::Timestamp) const = 0;
};

template <typename T>
class CallbackT:public Callback{
#ifndef NDEBUG
    //assert T 's base class is google::protobuf::Message
    BOOST_STATIC_ASSERT((boost::is_base_of<google::protobuf::Message,T>::value));
#endif
public:

    typedef boost::function
            <void(const muduo::net::TcpConnectionPtr &,
                  const boost::shared_ptr<T> &message, ///!! 注意这里的成员类型
                  muduo::Timestamp)> ProtobufMessageTCallback;

    //constructor set callback function
    CallbackT(const ProtobufMessageTCallback &callback):
            callback_(callback)
    {

    }
    virtual void OnMessage(const muduo::net::TcpConnectionPtr &conn,const MessagePtr &message,muduo::Timestamp receiveTime) const{
        //向下转换，将message 转换为T 类型
        boost::shared_ptr<T> concrete = muduo::down_pointer_cast<T>(message);
        assert(concrete != nullptr);
        //call callback function
        callback_(conn,concrete,receiveTime);
    }
private:
    //callback function
    ProtobufMessageTCallback callback_;
};

class ProtobufDispatcher{
public:
    typedef boost::function
            <void(const muduo::net::TcpConnectionPtr &,
                  const MessagePtr &message,
                  muduo::Timestamp)> ProtobufMessageCallback;

    explicit ProtobufDispatcher(const ProtobufMessageCallback &defaultcb):
            defaultCallback_(defaultcb){}

    void onProtobufMessage(const muduo::net::TcpConnectionPtr &conn,const MessagePtr &message,muduo::Timestamp receiveTime) const{
        //get callback function of given descriptor
        CallbackMap::const_iterator it = callbacks_.find(message->GetDescriptor());
        if(it != callbacks_.end()){
            //call inner function named OnMessage inside class Callback
            it->second->OnMessage(conn,message,receiveTime);
        }else{
            //cannot find, call default callback function
            defaultCallback_(conn,message,receiveTime);
        }
    }
    //register call back function, store descriptor,callback key-value pair into map
    template <typename T>
            void registerMessageCallback(const typename CallbackT<T>::ProtobufMessageTCallback &callback)
    {
        boost::shared_ptr<CallbackT<T>> pd(new CallbackT<T>(callback));
        callbacks_[T::descriptor()] = pd;
    }

private:
    typedef std::map<const google::protobuf::Descriptor *,boost::shared_ptr<Callback>> CallbackMap;
    CallbackMap callbacks_;
    ProtobufMessageCallback defaultCallback_;
};
#endif //PROTOBUF_CODEC_DISPATCHER_H
