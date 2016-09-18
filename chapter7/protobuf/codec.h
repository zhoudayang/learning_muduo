//
// Created by zhouyang on 16-9-18.
//

#ifndef PROTOBUF_CODEC_H
#define PROTOBUF_CODEC_H
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include <zlib.h>
#include <string>

#include <arpa/inet.h>
#include <stdint.h>

const int kHeaderLen = sizeof(int32_t);

//function to encode protobuf to string
//包括 protobuf 消息总长度 命名信息 命名长度 protobuf 字符串表示 校验值
inline std::string  encode(const google::protobuf::Message &message){
    std::string result;
    //提前开辟空间用于最后存储总长度
    result.resize(kHeaderLen);
    const std::string &typeName = message.GetTypeName();
    //名称的长度
    int32_t nameLen = static_cast<int32_t> (typeName.size() +1);
    int32_t be32 = ::htonl(nameLen);
    //namelen name
    result.append(reinterpret_cast<char*>(&be32),sizeof be32);
    result.append(typeName.c_str(),nameLen);
    //AppendToString method
    // Like SerializeToString(), but appends to the data to the string's existing
    // contents.  All required fields must be set.
    bool succeed = message.AppendToString(&result);
    //if convert fail, clear result, return empty string
    if(succeed){
        //真正开始的位置为protobuf message 长度之后的位置
        const char * begin = result.c_str() + kHeaderLen;
        //计算得到校验值
        int32_t checkSum = adler32(1,reinterpret_cast<const Bytef*>(begin),result.size()-kHeaderLen);
        int32_t be32 = ::htonl(checkSum);
        //加入校验值
        result.append(reinterpret_cast<char*>(&be32),sizeof be32);
        int32_t len = ::htonl(result.size() -kHeaderLen);
        //在string头部位置加入整个protobuf的总长度 包括命名 命名长度 消息字符串 校验值
        std::copy(reinterpret_cast<char*>(&len),reinterpret_cast<char*>(&len)+sizeof len,result.begin());
    }
    else{
        result.clear();
    }
    return result;
}

//根据type_name 创建 Message对象
inline google::protobuf ::Message * createMessage(const std::string & type_name){
    google::protobuf::Message * message = NULL;
    const google::protobuf::Descriptor * descriptor = google::protobuf::DescriptorPool::generated_pool() ->FindMessageTypeByName(type_name);
    if(descriptor){
        const google::protobuf::Message * prototype = google::protobuf::MessageFactory::generated_factory()->GetPrototype(descriptor);
        if(prototype){
            message = prototype -> New();
        }
    }
    return message;
}

inline int32_t asInt32(const char * buf){
    int32_t be32 = 0;
    ::memcpy(&be32,buf,sizeof(be32));
    return ::ntohl(be32);
}

inline google::protobuf::Message * decode(const std::string &buf){
    google::protobuf::Message  *result = NULL;
    int32_t len = static_cast<int32_t>(buf.size());
    if(len>10){
        int32_t expectedCheckSum = asInt32(buf.c_str()+buf.size()-kHeaderLen);
        const char * begin = buf.c_str();
        int32_t checkSum = adler32(1,reinterpret_cast<const Bytef *>(begin),len-kHeaderLen);
        if(checkSum == expectedCheckSum){
            int32_t nameLen = asInt32(buf.c_str());
            if(nameLen >=2 && nameLen <= len-2*kHeaderLen){
                std::string typeName(buf.begin() + kHeaderLen, buf.begin() + kHeaderLen + nameLen - 1);
                google::protobuf::Message * message = createMessage(typeName);
                if(message){
                    const char *data = buf.c_str()+ kHeaderLen + nameLen;
                    int32_t dataLen = len-nameLen -2* kHeaderLen;
                    if(message->ParseFromArray(data,dataLen)){
                        result = message;
                    }
                    else{
                        delete message;
                    }
                }
                else{
                    //unknown message type
                }
            }
            else{
                //invalid name len
            }
        }
        else{
            //check sum error
        }
    }
    return result;
}
#endif //PROTOBUF_CODEC_H
