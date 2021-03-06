//
// Created by zhouyang on 16-8-11.
//

#include "logging/LogFile.h"
#include "logging/logging.h"

boost ::scoped_ptr<muduo::LogFile> g_logFile;

//output function
void outputFunc(const char *msg,int len){
    g_logFile->append(msg,len);
}

//flush function
void flushFunc(){
    g_logFile->flush();
}
int main(){
    char name[256]="basic_logfile_name";
    g_logFile.reset(new muduo::LogFile(::basename(name),256*1024));
    muduo::Logger::setOutput(outputFunc);
    muduo::Logger::setFlush(flushFunc);
    muduo::string line = "1234567890 abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    for(int i=0;i<10000;i++)
    {
        LOG_INFO<<line;
        usleep(100);
    }

}