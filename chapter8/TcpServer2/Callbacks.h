//
// Created by zhouyang on 16/8/1.
//

#ifndef CALLBACKS_H
#define CALLBACKS_H

#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>

#include "datetime/Timestamp.h"

namespace muduo {
    //an client visibke callbacks go here
    // 定时器绑定的回调函数
    typedef boost::function<void()> TimerCallback;

    class TcpConnection;

    typedef boost::shared_ptr<TcpConnection> TcpConnectionPtr;
    //add connection callback function
    typedef boost::function<void(const TcpConnectionPtr &)> ConnectionCallback;
    //add message call back function
    typedef boost::function<void(const TcpConnectionPtr &,
                                 const char *data,
                                 ssize_t len)> MessageCallback;
    //close callback function
    typedef boost::function<void(const TcpConnectionPtr&)> CloseCallback;
}

#endif
