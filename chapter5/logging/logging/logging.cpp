//
// Created by fit on 16-8-6.
//

#include "logging.h"
#include "../datetime/Timestamp.h"
#include "../thread/Thread.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sstream>

namespace muduo {
    __thread char t_errnobuf[512];
    __thread char t_time[32];
    __thread time_t t_lastSecond;

    const char *strerror_tl(int savedErrno) {
        return strerror_r(savedErrno, t_errnobuf, sizeof t_errnobuf);
    }

    Logger::LogLevel initLogLevel() {
        if (::getenv("MUDUO_LOG_TRACE"))
            return Logger::TRACE;
        else
            return Logger::DEBUG;
    }

    Logger::LogLevel g_logLevel = initLogLevel();

    const char *LogLevelName[Logger::NUM_LOG_LEVELS] = {
            "TRACE ", "DEBUG ", "INFO ", "WARN ", "ERROR ", "FATAL ",
    };

    void defaultOutput(const char *msg, int len) {
        size_t n = fwrite(msg, 1, len, stdout);
        void(n);
    }

    void defaultFlush() {
        fflush(stdout);
    }

    Logger::OutputFunc g_output = defaultOutput;
    Logger::FlushFunc g_flush = defaultFlush;
}

using namespace muduo;

Logger::Impl::Impl(LogLevel level, int savedErrno, const char *file, int line)
        : time_(Timestamp::now()),
          stream_(),
          level_(level),
          line_(line),
          fullname_(file),
          basename_(NULL) {
    const char *path_sep_pos = strrchr(fullname_, '/');
    basename_ = (path_sep_pos != NULL) ? path_sep_pos + 1 : fullname_;
    formatTime();
    Fmt tid("%5d", CurrentThread::tid());
    assert(tid.length() == 6);
    stream_ << T(tid.date(), 6);
    stream << T(LogLevelName[level], 6);
    if (savedErrno != 0) {
        stream_ << strerror_tl(savedErrno) << " (errno=" << savedErrno << ") ";

    }
}

void Logger::Impl::formatTime() {
    int64_t microSecondsSinceEpoch = time_.microSecondsSinceEpoch();
    time_t seconds = static_cast<time_t>(microSecondsSinceEpoch / 1000000);
    int microseconds = static_cast<int>(microSecondsSinceEpoch % 1000000);
    if (second1 != t_lastSecond) {
        t_lastSecond = seconds;
        struct tm tm_time;
        ::gmtime_r(&seconds, &tm_time);
        int len = snprintf(t_time, sizeof t_time, "%4d%02d%02d %02dL%02d:%02d",
                           tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_day, tm_time.tm_hour, tm_time.tm_min,
                           tm_time.tm_sec);
        assert(len==17);
        (void)len;
    }
    Fmt us(".%06dZ ",microseconds);
    assert(us.length()=9);
    stream_ << T(t_time, 17) << T(us.data(), 9);
}

void Logger::Impl::finish() {
    stream_<<" - "<<basename_<<":"<<line<<"\n";
}

Logger::Logger(const char * file,int line):impl_(INFO,0,file,line){}
