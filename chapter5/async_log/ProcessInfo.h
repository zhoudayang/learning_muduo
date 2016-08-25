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
        pid_t pid();

        string pidString();

        uid_t uid();

        string username();

        uid_t euid();

        Timestamp startTime();

        int clockTicksPerSecond();

        int pageSize();

        bool isDebugBuild();

        string hostname();

        string procname();

        StringPiece procname(const string &stat);

        string procStatus();

        string threadStat();

        string exePath();

        int openedFiles();

        int maxOpenFiles();

        struct CpuTime {
            double userSeconds;
            double systemSeconds;

            CpuTime() : userSeconds(0.0), systemSeconds(0.0) { }
        };

        CpuTime cpuTime();

        int numThreads();

        std::vector<pid_t> threads();

    }
}


#endif
