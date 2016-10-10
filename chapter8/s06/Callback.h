//
// Created by zhouyang on 16-9-28.
//

#ifndef S02_CALLBACK_H
#define S02_CALLBACK_H

#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include "base/Timestamp.h"

namespace muduo {
    class TcpConnection;

    //TcpConnection shared_ptr
    typedef boost::shared_ptr<TcpConnection> TcpConnectionPtr;

    //Timer Callback function
    typedef boost::function<void()> TimerCallback;

    //connection callback function
    typedef boost::function<void(const TcpConnectionPtr &)> ConnectionCallback;

    //message callback function
    typedef boost::function<void(const TcpConnectionPtr &, const char *data, ssize_t len)> MessageCallback;

    //close callback function definition
    typedef boost::function<void(const TcpConnectionPtr &)> CloseCallback;

}
#endif //S02_CALLBACK_H
