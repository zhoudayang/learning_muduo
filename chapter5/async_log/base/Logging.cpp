#include "Logging.h"
#include "CurrentThread.h"
#include "Timestamp.h"
#include "TimeZone.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sstream>


namespace muduo {
    __thread char t_errnobuf[512];
    __thread char t_time[32];
    //cache last log second of time
    __thread time_t t_lastSecond;

    //store meaning of errno to t_errnobuf
    const char *strerror_tl(int savedErrno) {
        return strerror_r(savedErrno, t_errnobuf, sizeof(t_errnobuf));
    }

    Logger::LogLevel initLogLevel() {
        //if set env MUDUO_LOG_TRACE
        if (::getenv("MUDUO_LOG_TRACE"))
            //set log level to TRACE
            return Logger::TRACE;
        else if (::getenv("MUDUO_LOG_DEBUG"))
            return Logger::DEBUG;
        else
            return Logger::INFO;
    }

    //init log level
    Logger::LogLevel g_logLevel = initLogLevel();

    const char *LogLevelName[Logger::NUM_LOG_LEVELS] = {
            "TRACE ",
            "DEBUG ",
            "INFO  ",
            "WARN  ",
            "ERROR ",
            "FATAL ",
    };

    //helper class for known string length at compile time
    class T {
    public:
        T(const char *str, unsigned len) : str_(str), len_(len) {
            assert(strlen(str) == len);
        }

        const char *str_;
        const unsigned len_;
    };

    inline LogStream &operator<<(LogStream &s, T v) {
        s.append(v.str_, v.len_);
        return s;
    }

    //append source file name
    inline LogStream &operator<<(LogStream &s, const Logger::SourceFile &v) {
        s.append(v.data_, v.size_);
        return s;
    }

    //default output function, output to stdout
    void defaultOutput(const char *msg, int len) {
        size_t n = fwrite(msg, 1, len, stdout);
        (void) n;
    }

    //default stdout, flush stdout
    void defaultFlush() {
        ::fflush(stdout);
    }

    Logger::OutputFunc g_output = defaultOutput;
    Logger::FlushFunc g_flush = defaultFlush;
    TimeZone g_logTimeZone;
}

using namespace muduo;

Logger::Impl::Impl(LogLevel level, int savedErrno, const SourceFile &file, int line)
        : time_(Timestamp::now()),
          stream_(),
          level_(level),
          line_(line),
          basename_(file) {
    formatTime();
    CurrentThread::tid();
    //append tid string
    stream_ << T(CurrentThread::tidString(), CurrentThread::tidStringLength());
    //append log level name
    stream_ << T(LogLevelName[level], 6);
    if (savedErrno != 0) {
        //append error info
        stream_ << strerror_tl(savedErrno) << " (errno=" << savedErrno << ") ";
    }
}

//how to format time
void Logger::Impl::formatTime() {
    int64_t microSecondsSinceEpoch = time_.microSecondsSinceEpoch();
    time_t seconds = static_cast<time_t>(microSecondsSinceEpoch / Timestamp::kMicroSecondsPerSecond);
    int microseconds = static_cast<int>(microSecondsSinceEpoch % Timestamp::kMicroSecondsPerSecond);
    if (seconds != t_lastSecond) {
        t_lastSecond = seconds;
        struct tm tm_time;
        if (g_logTimeZone.valid()) {
            tm_time = g_logTimeZone.toLocalTime(seconds);
        } else {
            ::gmtime_r(&seconds, &tm_time); // FIXME TimeZone::fromUtcTime
        }

        int len = snprintf(t_time, sizeof(t_time), "%4d%02d%02d %02d:%02d:%02d",
                           tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
                           tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
        assert(len == 17);
        (void) len;
    }

    if (g_logTimeZone.valid()) {
        Fmt us(".%06d ", microseconds);
        assert(us.length() == 8);
        stream_ << T(t_time, 17) << T(us.data(), 8);
    } else {
        Fmt us(".%06dZ ", microseconds);
        assert(us.length() == 9);
        stream_ << T(t_time, 17) << T(us.data(), 9);
    }
}

//add finish string
void Logger::Impl::finish() {
    stream_ << " - " << basename_ << ":" << line_ << "\n";
}

Logger::Logger(SourceFile file, int line) : impl_(INFO, 0, file, line) {}

Logger::Logger(SourceFile file, int line, LogLevel level, const char *func) :
        impl_(level, 0, file, line) {
    //append function name
    impl_.stream_ << func << ' ';
}

Logger::Logger(SourceFile file, int line, LogLevel level) :
        impl_(level, 0, file, line) {

}

Logger::Logger(SourceFile file, int line, bool toAbort) :
        impl_(toAbort ? FATAL : ERROR, errno, file, line) {}

Logger::~Logger() {
    //call finish function, and finish string
    impl_.finish();
    const LogStream::Buffer &buf(stream().buffer());
    //output buffer
    g_output(buf.data(), buf.length());
    //if fatal, abort to exit the software
    if (impl_.level_ == FATAL) {
        g_flush();
        abort();
    }
}

void Logger::setLogLevel(LogLevel level) {
    g_logLevel = level;
}

void Logger::setOutput(OutputFunc func) {
    g_output = func;
}

void Logger::setFlush(FlushFunc func) {
    g_flush = func;
}

void Logger::setTimeZone(const TimeZone &tz) {
    g_logTimeZone = tz;
}
