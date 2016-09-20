//
// Created by zhouyang on 16-9-20.
//

#include "codec.h"
#include <muduo/base/Logging.h>
#include <muduo/net/Endian.h>
#include <muduo/net/protorpc/google-inl.h>

#include <google/protobuf/descriptor.h>
#include <zlib.h>

using namespace muduo;
using namespace muduo::net;

void ProtobufCodec::fillEmptyBuffer(muduo::net::Buffer *buf, const google::protobuf::Message &message) {

}

namespace {
    const string kNoErrorStr = "noError";
    const string kInvalidLengthStr = "InvalidLength";
    const string kCheckSumErrorStr = "CheckSumError";
    const string kInvalidNameLenStr = "InvalidNameLen";
    const string kUnknownMessageTypeStr = "UnknownMessageType";
    const string kParseErrorStr = "ParseError";
    const string kUnknownErrorStr = "UnknownError";
}

const string & ProtobufCodec::errorCodeToString(ErrorCode errorCode) {

}

vid ProtobufCodec::defaultErrorCallback(const muduo::net::TcpConnectionPtr & con, muduo::net::Buffer *buf, muduo::Timestamp,
                                        ErrorCode errorCode) {

}

int32_t asInt32(const char * buf){

}

void ProtobufCodec::onMessage(const muduo::net::TcpConnectionPtr &conn, muduo::net::Buffer *buf,
                              muduo::Timestamp receiveTime) {

}

google::protobuf::Message * ProtobufCodec::createMessage(const std::string &type_name) { }


MessagePtr ProtobufCodec::parse(const char *buf, int len, ErrorCode *errorCode) {

}
