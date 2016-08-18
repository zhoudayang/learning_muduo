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
    explicit File(const string &filename) :
            //open file  fp_ file_descriptor
            fp_(::fopen(filename.data(), "ae")),
            writtenBytes_(0)
    {
        //set file buffer
        ::setbuffer(fp_, buffer_, sizeof buffer_);
    }

    ~File() {//close file
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
        //add len to writtenBytes
        writtenBytes_ += len;
    }

    void flush() {
        //flush the file
        ::fflush(fp_);
    }
    //return written bytes
    size_t writtenBytes() const {
        return writtenBytes_;
    }

private:
    //write logline with len to fp_
    size_t write(const char *logline, size_t len) {
#undef fwrite_unlocked
        return ::fwrite_unlocked(logline, 1, len, fp_);
    }

    FILE *fp_;
    char buffer_[64 * 1024];
    size_t writtenBytes_;
};
LogFile::LogFile(const string &basename, size_t rollSize, bool threadSafe, int flushInterval)
:basename_(basename),
 rollSize_(rollSize),
 flushInterval_(flushInterval),
 count_(0),
 mutex_(threadSafe ? new MutexLock() : NULL),
 startOfPeriod(0),
 lastRoll_(0),
 lastFlush_(0)
{
    assert(basename.find('/')==string::npos);//在basename中没有/
    rollFile();
}
LogFile::~LogFile() {}

//!!调用入口
void LogFile ::append(const char *logline, int len) {
    // 需要保证线程安全
    if(mutex_){
        MutexLockGuard lock(*mutex_);
        append_unlocked(logline,len);
    }else{
        append_unlocked(logline,len);
    }
}

void LogFile::flush(){
    if(mutex_){
        MutexLockGuard lock(*mutex_);
        file_->flush();
    }
    else
        file_->flush();
}

//!! 关键入口
void LogFile::append_unlocked(const char *logline, int len) {
    file_->append(logline,len);
    //写入数量大于rollSize_ roll the log file
    if(file_->writtenBytes()>rollSize_){
        rollFile();
    }
    else{
        //写入数量大于kCheckTimeRoll_，需要检查是否需要roll file
        if(count_>kCheckTimeRoll_){
            count_=0;
            //::全局作用域符号
            time_t now =::time(NULL);
            time_t thisPeriod_ = now/kRollPerSeconds_*kRollPerSeconds_;
            //周期不同，进入了下一个周期 roll file
            if(thisPeriod_!=startOfPeriod){
                rollFile();
             // 超过最大flush 间隔 flush
            }else if(now -lastFlush_ >flushInterval_){
                lastFlush_ =now;
                file_->flush();
            }
        }else
        {
            ++count_;
        }
    }

}
void LogFile::rollFile(){
    time_t now = 0;
    string filename =getLogFileName(basename_,&now);
    time_t start = now/kRollPerSeconds_*kRollPerSeconds_;
    if(now>lastRoll_){
        lastRoll_ = now;
        lastFlush_ = now;
        startOfPeriod =start;
        //新建File对象
        file_.reset(new File(filename));
    }
}

//get log file name
string LogFile ::getLogFileName(const string &basename, time_t *now) {
    string filename;
    filename.reserve(basename.size()+32);
    filename = basename;
    char timebuf[32];
    char pidbuf[32];
    struct tm tm;
    *now = ::time(NULL);
    gmtime_r(now,&tm);
    strftime(timebuf,sizeof timebuf,".%Y%m%d-%H%M%S",&tm);
    filename+=timebuf;
    snprintf(pidbuf,sizeof pidbuf,".%d",::getpid());
    filename+=pidbuf;
    filename+=".log";
    return filename;
}