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
    //名称的长度,加上末尾的分隔符，还要加上1个字节
    int32_t nameLen = static_cast<int32_t> (typeName.size() +1);
    //将其转换为网络字节顺序
    int32_t be32 = ::htonl(nameLen);
    //namelen name
    //use reinterpret_cast good idea!
    result.append(reinterpret_cast<char*>(&be32),sizeof be32);
    /// append 实际内容的长度小于nameLen 自动 填充 0
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
    //首先由type_name 获取descriptor
    const google::protobuf::Descriptor * descriptor = google::protobuf::DescriptorPool::generated_pool() ->FindMessageTypeByName(type_name);
    if(descriptor){
        //再由descriptor 获取Message *
        const google::protobuf::Message * prototype = google::protobuf::MessageFactory::generated_factory()->GetPrototype(descriptor);
        if(prototype){
            //通过New() 方法获取新建Message * 实例
            message = prototype -> New();
        }
    }
    return message;
}

//从buffer的头部得到protobuf编码消息的长度
inline int32_t asInt32(const char * buf){
    int32_t be32 = 0;
    ::memcpy(&be32,buf,sizeof(be32));
    return ::ntohl(be32);
}

inline google::protobuf::Message * decode(const std::string &buf){
    google::protobuf::Message  *result = NULL;
    int32_t len = static_cast<int32_t>(buf.size());
    //如果消息长度大于等于最小长度10, 那么进行消息解码
    if(len>10){
        //get checkSum number
        int32_t expectedCheckSum = asInt32(buf.c_str()+buf.size()-kHeaderLen);
        const char * begin = buf.c_str();
        // 计算当前消息的校验码
        int32_t checkSum = adler32(1,reinterpret_cast<const Bytef *>(begin),len-kHeaderLen);
        //如果计算得到的校验码和消息中的校验码内容一致
        if(checkSum == expectedCheckSum){
            int32_t nameLen = asInt32(buf.c_str());
            //只有nameLen的长度大于或者等于2,typename才有值
            if(nameLen >=2 && nameLen <= len-2*kHeaderLen){
                ///get protobuf typename 将'\0'省去，所以需要减去1
                std::string typeName(buf.begin() + kHeaderLen, buf.begin() + kHeaderLen + nameLen - 1);
                google::protobuf::Message * message = createMessage(typeName);
                //if get valid message
                if(message){
                    //begin of data
                    const char *data = buf.c_str()+ kHeaderLen + nameLen;
                    //protobuf data length
                    int32_t dataLen = len-nameLen -2* kHeaderLen;
                    //parse to protobuf instance
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
