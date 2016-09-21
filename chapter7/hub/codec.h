//
// Created by zhouyang on 16-7-27.
//

#ifndef HUB_CODEC_H
#define HUB_CODEC_H

#include <muduo/base/Types.h>
#include <muduo/net/Buffer.h>

#include <boost/noncopyable.hpp>

namespace pubsub {

    using muduo::string;

    //enum to show status
    enum ParseResult {
        kError, //error
        kSuccess, //success
        kContinue,//continue
    };

    ParseResult parseMessage(muduo::net::Buffer *buf,
                             string *cmd,
                             string *topic,
                             string *content);
}
#endif //HUB_CODEC_H
