//
// Created by fit on 16-8-25.
//

#ifndef LOGGING_H
#define LOGGING_H

#include "LogStream.h"
#include "Timestamp.h"

namespace muduo {
    class TimeZone;

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

        class SourceFile {
        public:
            template<int N>
            inline SourceFile(const char (&arr)[N]):data_(arr), size_(N - 1) {
                const char *slash = strrchr(data_, '/');
                if (slash) {
                    data_ = slash + 1;
                    size_ -= static_cast<int>(data_ - arr);
                }
            }

            explicit SourceFile(const char *filename) : data_(filename) {
                const char *slash = strrchr(filename, '/');
                if (slash) {
                    data_ = slash;
                }
                size_ = static_cast<int>(strlen(data_));
            }


            const char *data_;
            int size_;
        };

        Logger(SourceFile file, int line);

        Logger(SourceFile file, int line, LogLevel level);

        Logger(SourceFile file, int line, LogLevel level, const char *func);

        Logger(SourceFile file, int line, bool toAbort);

        ~Logger();

        LogStream &stream() {
            return impl_.stream_;
        }

        static LogLevel loglevel();

        static void setLogLevel(LogLevel level);

        typedef void(*OutputFunc)(const char *msg, int len);

        typedef void(*FlushFunc)();

        static void setOutput(OutputFunc);

        static void setFlush(FlushFunc);

        static void setTimeZone(const TimeZone &tz);

    private:
        class Impl {
        public:
            typedef Logger::LogLevel LogLevel;

            Impl(LogLevel level, int savedErrno, const SourceFile &file, int line);

            void formatTime();

            void finish();

            Timestamp time_;
            LogStream stream_;
            LogLevel level_;
            int line_;
            SourceFile basename_;
        };

        Impl impl_;

    };

    extern Logger::LogLevel g_logLevel;

    inline Logger::LogLevel Logger::loglevel() {
        return g_logLevel;
    }

#define LOG_TRACE if(muduo::Logger::LogLevel() <= muduo::Logger::TRACE)\
    muduo::Logger(__FILE__,__LINE__,muduo::Logger::TRACE,__func__).stream()
#define LOG_DEBUG if(muduo::Logger::LogLevel()<=muduo::Logger::DEBUG)\
    muduo::Logger(__FILE__,__LINE__,muduo::Logger::DEBUG,__func__).stream()
#define LOG_INFO if(muduo::Logger::LogLevel()<=muduo::Logger::INFO)\
    muduo::Logger(__FILE__,__LINE__).stream()
#define LOG_WARN muduo::Logger(__FILE__,__LINE__,muduo::Logger::WARN).stream()
#define LOG_ERROR muduo::Logger(__FILE__,__LINE__,muduo::Logger::ERROR).stream()
#define LOG_FATAL muduo::Logger(__FILE__,__LINE__,muduo::Logger::FATAL).stream()
#define LOG_SYSERR muduo::Logger(__FILE__,__LINE__,false).stream()
#define LOG_SYSFATAL muduo::Logger(__FILE__,__LINE__,true).stream()

#define CHECK_NOTNULL(val)\
::muduo::CehckNotNull(__FILE__,__LINE__,"'"#val"'Must be non NULL",(val))

    const char* strerror_tl(int savedErrno);

    //A small helper for CHECK_NOTNULL()
    template<typename T>
    T *CheckNotNull(Logger::SourceFile file, int line, const char *names, T *ptr) {
        if (ptr == NULL) {
            Logger(file, line, Logger::FATAL).stream() << names;
        }
        return ptr;
    }

}


#endif
