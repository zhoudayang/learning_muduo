#include "codec.h"
#include "query.pb.h"

#include <stdio.h>

void print(const std::string &buf){
    printf("encoded to %zd bytes \n",buf.size());
    for(auto i=0;i<buf.size();i++){
        printf("%2zd: 0x%02x  %c\n", i, (unsigned char) buf[i], isgraph(buf[i]) ? buf[i]: ' ');
    }
}

void testQuery(){
    muduo::Query query;
    query.set_id(1);
    query.set_questioner("zhou yang");
    query.add_question("running");

    std::string transport = encode(query);
    print(transport);

}