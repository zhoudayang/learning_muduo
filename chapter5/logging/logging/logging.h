//
// Created by fit on 16-8-6.
//

#ifndef LOGGING_H
#define LOGGING_H

#include "LogStream.h"
#include "datetime/Timestamp.h"
#include <boost/scoped_ptr.hpp>

namespace muduo {
    class Logger {
    public:
        enum LogLevel {
            TRACE,
            DEBUG,
            INFO,
            WARN,
            ERROR,
            FATAL,
            NUM_LOG_LEVELS,
        };

        Logger(const char *file, int line);

        Logger(const char *file, int line, LogLevel level);

        Logger(const char *file, int line, LogLevel level, const char *func);

        Logger(const char *file, int line, bool toAbort);

        ~Logger();

        LogStream &stream() {
            return impl_.stream_;
        }

        static LogLevel logLevel();

        static void setLogLevel(LogLevel level);

        typedef void (*OutputFunc)(const char *msg, int len);

        typedef void (*FlushFunc)();

        typedef void(*OutputFunc)(const char *msg, inr len);

        typedef void(*FlushFunc)();

        static void setOutput(OutputFunc);

        static void setFlush(FlushFunc);

    private:
        class Impl {
        public:
            typedef Logger::LogLevel logLevel;

            Impl(LogLevel level, int old_errno, const char *file, int line);

            void formatTime();

            void finish();

            Timestamp time_;
            LogStream stream_;
            LogLevel level_;
            int line_;
            const char *fullname_;
            const char *basename_;
        };

        Impl impl_;
    };

#define LOG_TRACE if(muduo::Logger::logLevel()<=muduo::Logger::TRACE) \
    muduo::Logger(__FILE__,__LINE__,muduo::Logger::TRACE,__func__).stream()
#define LOG_DEBUG if(muduo::Logger::logLevel()<=muduo::Logger::DEBUG) \
     muduo::Logger(__FILE__,__LINE__,muduo::Logger::DEBUG,__func__).stream()
#define LOG_INFO if(muduo::Logger::logLevel()<=muduo::Logger::INFO) \
    muduo::Logger(__FILE__,__LINE__,muduo::Logger::INFO,__func__).stream()
#define LOG_WARN muduo::Logger(__FILE__,__LINE__,muduo::Logger::WARN).stream()
#define LOG_ERROR muduo::Logger(__FILE__,__LINE__,muduo::Logger::ERROR).stream()
#define LOG_FATAL muduo::Logger(__FILE__,__LINE__,muduo::Logger::FATAL).stream()
#define LOG_SYSERR muduo::Logger(__FILE__,__LINE__,false).stream()
#define LOG_SYSFATAL muduo::Logger(__FILE__,__LINE__,true).stream()

    const char *strerror_tl(int savedErrno);

//!!此处"#val"表明　引入字符串　val
#define CHECK_NOTNULL(val)\
    ::muduo::CheckNotNull(__FILE__,__LINE__,"'" #val "' Must be non NULL",(val))

    template<typename T>
    T *CheckNotNull(const char *file, int line, const char *names, T *ptr) {
        if (ptr == NULL) {
            Logger(file, line, Logger::FATAL).stream() << names;
        }
        return ptr;
    }

    template<typename To, typename From>
    inline To implicit_cast(From const &f) {
        return f;
    };
}


#endif //LOGGING_H
