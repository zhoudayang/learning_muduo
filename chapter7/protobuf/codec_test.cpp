#include "codec.h"
#include "query.pb.h"
#include <stdio.h>
#include <iostream>

using namespace std;


void print(const std::string &buf){
    printf("encoded to %zd bytes \n",buf.size());
    for(auto i=0;i<buf.size();i++){
        printf("%2zd: 0x%02x  %c\n", i, (unsigned char) buf[i], isgraph(buf[i]) ? buf[i]: ' ');
    }
}
/*
    required int64 id =1;
    required string questioner =2 ;
    repeated string question =3;
 */
void testQuery(){
    muduo::Query query;
    query.set_id(1);
    query.set_questioner("zhou yang");
    query.add_question("Running?");

    std::string transport = encode(query);
    print(transport);
    int32_t be32 =0;
    std::copy(transport.begin(),transport.begin() + sizeof be32,reinterpret_cast<char * >(&be32));
    //获取protobuf封装之后的消息长度
    int32_t len = ::ntohl(be32);
    assert(len == transport.size()  - sizeof be32);
    std::string buf = transport.substr(sizeof(int32_t));
    assert(len == buf.size());

    muduo::Query * newQuery = dynamic_cast<muduo::Query *>(decode(buf));
    assert(newQuery != NULL);
    cout<<newQuery->id()<<endl;
    cout<<newQuery->questioner()<<endl;
    int size = newQuery->question_size();
    cout<<size<<endl;
    for(int i=0;i<size;i++)
        cout<<newQuery->question(i)<<endl;
    delete newQuery;
    buf[buf.size() -6]++;
    muduo::Query * badQuery = dynamic_cast<muduo::Query*>(decode(buf));
    assert(badQuery ==NULL);


}

//codec test pass

int main(){

    testQuery();

    return 0;
}