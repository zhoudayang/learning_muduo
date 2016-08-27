//
// Created by zhouyang on 16-8-25.
//

#ifndef ASYNC_LOG_PROCESSINFO_H
#define ASYNC_LOG_PROCESSINFO_H

#include "StringPiece.h"
#include "Types.h"
#include "Timestamp.h"
#include <vector>

namespace muduo {
    namespace ProcessInfo {
        //查看进程pid
        pid_t pid();

        //将进程pid转换为　string
        string pidString();

        //获取用户id
        uid_t uid();

        //获取用户名
        string username();

        //获取用户euid
        uid_t euid();

        //获取启动时间
        Timestamp startTime();

        int clockTicksPerSecond();

        int pageSize();

        //is build with debug on ?
        bool isDebugBuild();

        string hostname();

        string procname();

        StringPiece procname(const string &stat);

        string procStatus();

        string threadStat();

        string exePath();

        //获取打开的文件数量
        int openedFiles();

        int maxOpenFiles();

        struct CpuTime {
            double userSeconds;
            double systemSeconds;

            CpuTime() : userSeconds(0.0), systemSeconds(0.0) {}
        };

        CpuTime cpuTime();

        int numThreads();

        std::vector<pid_t> threads();

    }
}


#endif
