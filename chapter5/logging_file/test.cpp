//
// Created by fit on 16-8-6.
//

#include "logging/logging.h"



int main() {
    LOG_TRACE << "trace";
    LOG_DEBUG << "debug";
    LOG_INFO << "Hello";
    LOG_WARN << "World";
    LOG_ERROR << "Error";
    LOG_INFO << sizeof(muduo::Logger);
    LOG_INFO << sizeof(muduo::LogStream);
    LOG_INFO << sizeof(muduo::Fmt);
    LOG_INFO << sizeof(muduo::LogStream::Buffer);


}
