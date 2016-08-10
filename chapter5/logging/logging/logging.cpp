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
        //设置了环境变量为TRACE
        if (::getenv("MUDUO_LOG_TRACE"))
            return Logger::TRACE;
        else
            return Logger::DEBUG;
    }

    Logger::LogLevel g_logLevel = initLogLevel();

    const char *LogLevelName[Logger::NUM_LOG_LEVELS] = {
            "TRACE ",
            "DEBUG ",
            "INFO  ",
            "WARN  ",
            "ERROR ",
            "FATAL ",
    };
    /*
     * size_t fwrite(const void* buffer, size_t size, size_t count, FILE* stream);
        注意：这个函数以二进制形式对文件进行操作，不局限于文本文件
        返回值：返回实际写入的数据块数目
        （1）buffer：是一个指针，对fwrite来说，是要获取数据的地址；
        （2）size：要写入内容的单字节数；
        （3）count:要进行写入size字节的数据项的个数；
        （4）stream:目标文件指针；
        （5）返回实际写入的数据项个数count。
     */
    void defaultOutput(const char *msg, int len) {
        size_t n = fwrite(msg, 1, len, stdout);
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
    //找到最后一个／字符指针
    const char *path_sep_pos = strrchr(fullname_, '/');
    basename_ = (path_sep_pos != NULL) ? path_sep_pos + 1 : fullname_;
    formatTime();
    Fmt tid("%5d ", CurrentThread::tid());
    assert(tid.length() == 6);
    stream_ << T(tid.data(), 6);
    stream_ << T(LogLevelName[level], 6);
    if (savedErrno != 0) {
        stream_ << strerror_tl(savedErrno) << " (errno=" << savedErrno << ") ";

    }
}

void Logger::Impl::formatTime() {
    int64_t microSecondsSinceEpoch = time_.microSecondsSinceEpoch();
    time_t seconds = static_cast<time_t>(microSecondsSinceEpoch / 1000000);
    int microseconds = static_cast<int>(microSecondsSinceEpoch % 1000000);
    //时间分为秒和微秒
    if (seconds != t_lastSecond) {
        t_lastSecond = seconds;
        struct tm tm_time;
        /*
            Return the `struct tm' representation of *TIMER in UTC,
            using *TP to store the result.
            extern struct tm *gmtime_r (const time_t *__restrict __timer,
                                    struct tm *__restrict __tp) __THROW;
         */
        ::gmtime_r(&seconds, &tm_time);
        int len = snprintf(t_time, sizeof t_time, "%4d%02d%02d %02d:%02d:%02d",
                           tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday, tm_time.tm_hour, tm_time.tm_min,
                           tm_time.tm_sec);
        assert(len == 17);
        (void) len;
    }
    Fmt us(".%06dZ ", microseconds);
    assert(us.length() == 9);
    stream_ << T(t_time, 17) << T(us.data(), 9);
}

void Logger::Impl::finish() {
    stream_ << " - " << basename_ << ":" << line_ << "\n";
}

Logger::Logger(const char *file, int line) : impl_(INFO, 0, file, line) {}

Logger::Logger(const char *file, int line, LogLevel level, const char *func)
        : impl_(level, 0, file, line) {
    impl_.stream_ << func << ' ';

}

Logger::Logger(const char *file, int line, LogLevel level) : impl_(level, 0, file, line) {}

Logger::Logger(const char *file, int line, bool toAbort)
        : impl_(toAbort ? FATAL : ERROR, errno, file, line) {}

//!!在析构函数中做出finish处理
//有几次调用构造函数就有几次调用析构函数，因为新建的对象没有引用，所以立即进行析构处理了
Logger::~Logger() {
    impl_.finish();
    const LogStream::Buffer &buf(stream().buffer());
    g_output(buf.data(), buf.length());
    if (impl_.level_ == FATAL) {
        g_flush();
        abort();
    }
}

Logger::LogLevel Logger::logLevel() {
    return g_logLevel;
}

void Logger::setLogLevel(LogLevel level) {
    g_logLevel = level;
}

void Logger::setOutput(OutputFunc out) {
    g_output = out;
}

void Logger::setFlush(FlushFunc flush) {
    g_flush = flush;
}
