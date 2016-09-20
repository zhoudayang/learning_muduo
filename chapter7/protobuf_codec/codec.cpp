//
// Created by zhouyang on 16-9-20.
//

#include "codec.h"
#include "google-inl.h"
#include <muduo/base/Logging.h>
#include <muduo/net/Endian.h>
#include <google/protobuf/descriptor.h>
#include <zlib.h>
#include <assert.h>

using namespace muduo;
using namespace muduo::net;

//向空的buffer中填充message 数据
void ProtobufCodec::fillEmptyBuffer(muduo::net::Buffer *buf, const google::protobuf::Message &message) {

    // buf->retrieveAll();
    assert(buf->readableBytes() == 0);

    const std::string& typeName = message.GetTypeName();
    int32_t nameLen = static_cast<int32_t>(typeName.size()+1);
    buf->appendInt32(nameLen);
    buf->append(typeName.c_str(), nameLen);

    // code copied from MessageLite::SerializeToArray() and MessageLite::SerializePartialToArray().
    // 判断message 是否已经初始化
    GOOGLE_DCHECK(message.IsInitialized()) << InitializationErrorMessage("serialize", message);

    int byte_size = message.ByteSize();
    //在buf中开辟大小为byte_size 的空间
    buf->ensureWritableBytes(byte_size);

    uint8_t* start = reinterpret_cast<uint8_t*>(buf->beginWrite());
    //将message 写入buf
    uint8_t* end = message.SerializeWithCachedSizesToArray(start);
    if (end - start != byte_size)
    {
        //错误处理函数
        ByteSizeConsistencyError(byte_size, message.ByteSize(), static_cast<int>(end - start));
    }
    buf->hasWritten(byte_size);

    int32_t checkSum = static_cast<int32_t>(
            ::adler32(1,
                      reinterpret_cast<const Bytef*>(buf->peek()),
                      static_cast<int>(buf->readableBytes())));
    buf->appendInt32(checkSum);
    assert(buf->readableBytes() == sizeof nameLen + nameLen + byte_size + sizeof checkSum);
    int32_t len = sockets::hostToNetwork32(static_cast<int32_t>(buf->readableBytes()));
    //在头部位置写入整个protobuf 封装消息的长度
    buf->prepend(&len, sizeof len);
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

//返回错误码对应的字符串描述
const string & ProtobufCodec::errorCodeToString(ErrorCode errorCode) {
    switch (errorCode){
        case kNoError:
            return kNoErrorStr;
        case kInvalidLength:
            return kInvalidLengthStr;
        case kCheckSumError:
            return kCheckSumErrorStr;
        case kInvalidNameLen:
            return kInvalidNameLenStr;
        case kUnKnownMessageType:
            return kUnknownMessageTypeStr;
        case kParseError:
            return kParseErrorStr;
        default:
            return kUnknownErrorStr;
    }
}

//默认的错误处理函数，直接断开连接
void ProtobufCodec::defaultErrorCallback(const muduo::net::TcpConnectionPtr & conn, muduo::net::Buffer *buf, muduo::Timestamp,
                                        ErrorCode errorCode) {
    LOG_ERROR<<"ProtobufCodec::defaultErrorCallback - "<<errorCodeToString(errorCode);
    if(conn && conn->connected()){
        conn->shutdown();
    }
}

//将buf中的字符串转换为int32_t数
int32_t asInt32(const char * buf){
    int32_t be32 = 0;
    ::memcpy(&be32,buf,sizeof(be32));
    return sockets::networkToHost32(be32);
}

void ProtobufCodec::onMessage(const muduo::net::TcpConnectionPtr &conn, muduo::net::Buffer *buf,
                              muduo::Timestamp receiveTime) {
    while(buf->readableBytes() >= kMinMessageLen + kHeaderLen){
        const int32_t len = buf->peekInt32();
        //invalid length
        if(len > kMaxMessageLen || len <kMinMessageLen){
            errorCallback_(conn,buf,receiveTime,kInvalidLength);
            break;
        }
        else if(buf->readableBytes() >= implicit_cast<size_t>(len + kHeaderLen)){
            ErrorCode errorCode = kNoError;
            MessagePtr  message = parse(buf->peek() + kHeaderLen, len ,&errorCode);
            //no error and valid message
            if(errorCode == kNoError && message){
                messageCallback_(conn,message,receiveTime);
                //内容读取完毕，重置读指针位置
                buf->retrieve(kHeaderLen+len);
            }
                //error, call error callback function
            else{
                errorCallback_(conn,buf,receiveTime,errorCode);
                break;
            }
        }
        else{
            break;
        }
    }
}
//use type_name to create message instance
google::protobuf::Message * ProtobufCodec::createMessage(const std::string &type_name) {
    google::protobuf::Message * message = NULL;
    const google::protobuf::Descriptor * descriptor = google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName(type_name);
    if(descriptor){
        const google::protobuf::Message * prototype =
        google::protobuf::MessageFactory::generated_factory()->GetPrototype(descriptor);
        if(prototype){
            message = prototype->New();
        }
    }
    return message;
}


MessagePtr ProtobufCodec::parse(const char *buf, int len, ErrorCode *error) {
    MessagePtr message;
    int32_t expectedCheckSum = asInt32(buf+len-kHeaderLen);
    int32_t checkSum = static_cast<int32_t>(::adler32(1, reinterpret_cast<const Bytef*>(buf),static_cast<int>(len-kHeaderLen)));
    //校验码检查通过
    if(checkSum ==expectedCheckSum) {
        int32_t nameLen = asInt32(buf);
        if (nameLen >= 2 && nameLen <= len - 2 * kHeaderLen) {
            std::string typeName(buf + kHeaderLen, buf + kHeaderLen + nameLen - 1);
            //create new message
            message.reset(createMessage(typeName));
            if (message) {
                const char *data = buf + kHeaderLen + nameLen;
                int32_t dataLen = len - nameLen - 2 * kHeaderLen;
                if (message->ParseFromArray(data, dataLen)) {
                    *error = kNoError;
                }//parse error
                else {
                    *error = kParseError;
                }
            }
            else {
                *error = kUnKnownMessageType;
            }
        }
        else {
            *error = kInvalidNameLen;
        }
    }//校验码检查失败
    else{
        *error = kCheckSumError;
    }
    return message;
}
