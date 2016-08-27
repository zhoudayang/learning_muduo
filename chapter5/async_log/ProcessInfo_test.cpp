
#include "base/ProcessInfo.h"
#include <stdio.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

//测试通过
int main(){
    printf("pid = %d\n",muduo::ProcessInfo::pid());
    printf("uid = %d \n",muduo::ProcessInfo::uid());
    printf("euid = %d \n",muduo::ProcessInfo::euid());
    printf("start time = %s \n",muduo::ProcessInfo::startTime().toFormattedString().c_str());
    printf("hostname = %s \n",muduo::ProcessInfo::hostname().c_str());
    printf("opend files = %d \n",muduo::ProcessInfo::openedFiles());
    printf("threads %zd \n",muduo::ProcessInfo::threads().size());
    printf("num threads = %d \n",muduo::ProcessInfo::numThreads());
    printf("status = %s \n",muduo::ProcessInfo::procStatus().c_str());
    return 0;
}