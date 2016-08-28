//
// Created by zhouyang on 16-8-25.
//

#include "LogFile.h"
#include "FileUtil.h"
#include "ProcessInfo.h"

#include <assert.h>
#include <stdio.h>
#include <time.h>

using namespace muduo;

LogFile::LogFile(const string &basename, size_t rollSize, bool threadSafe, int flushInterval, int checkEveryN)
        : basename_(basename),
          rollSize_(rollSize),
          flushInterval_(flushInterval),
          checkEveryN_(checkEveryN),
          count_(0),
          mutex_(threadSafe ? new MutexLock:NULL),
          startOfPeriod_(0),
          lastRoll_(0),
          lastFlush_(0) {
    //断言basename 中没有 '/'
    assert(basename.find('/') == string::npos);
    rollFile();
}

LogFile::~LogFile() {

}

//output function
void LogFile::append(const char *logline, int len) {
    if (mutex_) {
        MutexLockGuard lock(*mutex_);
        append_unlocked(logline, len);
    } else {
        append_unlocked(logline, len);
    }
}

//flush function
void LogFile::flush() {
    if (mutex_) {
        MutexLockGuard lock(*mutex_);
        file_->flush();
    } else {
        file_->flush();
    }
}

void LogFile::append_unlocked(const char *logline, int len) {
    file_->append(logline, len);
    if (file_->writtenBytes() > rollSize_) {
        rollFile();
    } else {
        ++count_;
        //写入的log数目达到检查界限
        if (count_ >= checkEveryN_) {
            count_ = 0;
            time_t now = ::time(NULL);
            time_t thisPeriod_ = now / kRollPerSeconds * kRollPerSeconds;
            if (thisPeriod_ != startOfPeriod_) {
                rollFile();
            }
            //达到flush间隔 flush file
            else if (now - lastFlush_ > flushInterval_) {
                lastFlush_ = now;
                file_->flush();
            }
        }
    }
}

//roll to new file
bool LogFile::rollFile() {
    time_t now = 0;
    string filename = getLogFileName(basename_, &now);
    time_t start = now / kRollPerSeconds * kRollPerSeconds;
    if (now > lastRoll_) {
        lastRoll_ = now;
        lastFlush_ = now;
        startOfPeriod_ = start;
        //reset new AppendFile object
        file_.reset(new FileUtil::AppendFile(filename));
        return true;
    }
    return false;
}

//get log file name
string LogFile::getLogFileName(const string &basename, time_t *now) {
    string filename;
    filename.reserve(basename.size() + 64);
    filename = basename;
    char timebuf[32];
    struct tm tm;
    *now = time(NULL);
    gmtime_r(now, &tm);
    strftime(timebuf, sizeof timebuf, ".%Y%m%d-%H%M%S.", &tm);
    filename += timebuf;
    filename += ProcessInfo::hostname();
    char pidbuf[32];
    snprintf(pidbuf, sizeof pidbuf, ".%d", ProcessInfo::pid());
    filename += pidbuf;
    filename += ".log";
    return filename;
}