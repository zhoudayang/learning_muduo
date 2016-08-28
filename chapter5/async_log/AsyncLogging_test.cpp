#include "base/AsyncLogging.h"
#include "base/Logging.h"
#include "base/Timestamp.h"

#include <stdio.h>
#include <sys/resource.h>


int kRollSize = 500 * 1000 * 1000;

muduo::AsyncLogging *g_asyncLog = NULL;

void asyncOutput(const char *msg, int len) {
    g_asyncLog->append(msg, len);
}

void bench(bool longLog) {
    //set asyncOutput as output function
    muduo::Logger::setOutput(asyncOutput);

    int cnt = 0;
    const int kBatch = 1000;
    muduo::string empty = " ";
    muduo::string longStr(3000, 'X');
    longStr += " ";

    for (int t = 0; t < 30; ++t) {
        muduo::Timestamp start = muduo::Timestamp::now();
        for (int i = 0; i < kBatch; ++i) {
            LOG_INFO << "Hello 0123456789" << " abcdefghijklmnopqrstuvwxyz "
                     << (longLog ? longStr : empty)
                     << cnt;
            ++cnt;
        }
        muduo::Timestamp end = muduo::Timestamp::now();
        printf("%f\n", timeDifference(end, start) * 1000000 / kBatch);
        //sleep for 500 ms
        struct timespec ts = {0, 500 * 1000 * 1000};
        nanosleep(&ts, NULL);
    }
}

int main(int argc, char *argv[]) {
    {
        // set max virtual memory to 2GB.
        size_t kOneGB = 1000 * 1024 * 1024;
        rlimit rl = {2 * kOneGB, 2 * kOneGB};
        /* Set the soft and hard limits for RESOURCE to *RLIMITS.
           Only the super-user can increase hard limits.
           Return 0 if successful, -1 if not (and sets errno).  */
        //RLIMIT_AS -> 进程的最大虚内存空间，字节为单位。
        setrlimit(RLIMIT_AS, &rl);
    }

    printf("pid = %d\n", getpid());

    char name[256];
    strncpy(name, argv[0], 256);
    muduo::AsyncLogging log(::basename(name), kRollSize);
    //start threadFunc in AsyncLogging class
    log.start();
    //set global log to log
    g_asyncLog = &log;
    //set longLog to true if argc > 1
    bool longLog = argc > 1;
    bench(longLog);
}

