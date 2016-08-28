//
// Created by zhouyang on 16-8-25.
//

#ifndef LOGFILE_H
#define LOGFILE_H

#include "Mutex.h"
#include "Types.h"
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>


namespace muduo {
    namespace FileUtil {
        //pre declare AppendFile class
        class AppendFile;
    }
    class LogFile : boost::noncopyable {
    public:
        LogFile(const string &basename, size_t rollSize, bool threadSafe = true, int flushInterval = 3,
                int checkEveryN = 1024);

        ~LogFile();

        void append(const char *logline, int len);

        void flush();

        bool rollFile();

    private:
        void append_unlocked(const char *logline, int len);

        static string getLogFileName(const string &basename, time_t *now);

        const string basename_;
        const size_t rollSize_;
        const int flushInterval_;
        const int checkEveryN_;

        int count_;
        boost::scoped_ptr<MutexLock> mutex_;
        time_t startOfPeriod_;
        time_t lastRoll_;
        time_t lastFlush_;
        boost::scoped_ptr<FileUtil::AppendFile> file_;

        const static int kRollPerSeconds = 60 * 60 * 24;
    };
}


#endif
