//
// Created by fit on 16-8-10.
//

#include "LogFile.h"

#include <assert.h>
#include <stdio.h>
#include <time.h>

using namespace muduo;

class LogFile::File : boost::noncopyable {
public:
    explicit File(const string &filename) : fp_(::fopen(filename.data(), "ae")), writtenBytes_(0) {
        ::setbuf(fp_, buffer_, sizeof buffer_);
    }

    ~File() {
        ::fclose(fp_);
    }

    void append(const char *logline, const size_t len) {
        size_t n = write(logline, len);
        size_t remain = len - n;
        while (remain > 0) {
            size_t x = write(logline + n, remain);
            if (x == 0) {
                int err = ferror(fp_);
                if (err) {
                    char buf[128];
                    strerror_r(err, buf, sizeof buf);
                    fprintf(stderr, "LogFile::File::append() failed %s\n", buf);

                }
                break;
            }
            n += x;
            remain = len - n;
        }
        writtenBytes_ += len;
    }

    void flush() {
        ::fflush(fp_);
    }

    size_t writtenBytes() const {
        return writtenBytes_;
    }

private:
    size_t write(const char *logline, size_t len) {
#undef fwrite_unlocked
        return ::fwrite_unlocked(logline, 1, len, fp_);
    }

    FILE *fp_;
    char buffer_[64 * 1024];
    size_t writtenBytes_;
};

LogFile::LogFile(const string &basename, size_t rollSize, bool threadSafe, int flushInterval)
:basename_(basename),rollSize_(rollSize),flushInterval_(flushInterval),count_(0),
 start

{}