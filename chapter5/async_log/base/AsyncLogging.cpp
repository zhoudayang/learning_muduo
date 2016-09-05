//
// Created by zhouyang on 16-8-25.
//

#include "AsyncLogging.h"
#include "Timestamp.h"
#include "LogFile.h"

#include <stdio.h>

using namespace muduo;

AsyncLogging::AsyncLogging(const string &basename, size_t rollSize, int flushInterval)
        : flushInterval_(flushInterval),
          running_(false),
          basename_(basename),
          rollSize_(rollSize),
          //bind AsyncLogging::threadFunc to thread
          thread_(boost::bind(&AsyncLogging::threadFunc, this), "Logging"),
          latch_(1),
          mutex_(),
          cond_(mutex_),
          currentBuffer_(new Buffer),
          nextBuffer_(new Buffer),
          buffers_()
{

    currentBuffer_->bzero();
    nextBuffer_->bzero();
    //set size of buffers_ to 16
    buffers_.reserve(16);
}
//may be called by multi thread!
void AsyncLogging::append(const char *logline, int len) {
    muduo::MutexLockGuard lock(mutex_);
    //如果当前的buffer剩余空间足够，讲logline 加入buffer中
    if (currentBuffer_->avail() > len) {
        currentBuffer_->append(logline, len);
    } else {
        buffers_.push_back(currentBuffer_.release());
        if (nextBuffer_) {
            //move next Buffer to replace current buffer
            currentBuffer_ = boost::ptr_container::move(nextBuffer_);
        }
        else
        {
            //reset current buffer to new buffer
            currentBuffer_.reset(new Buffer);
        }
        currentBuffer_->append(logline, len);
        //notify threadFunc write currentBuffer_ to file
        cond_.notify();
    }
}

void AsyncLogging::threadFunc() {
    assert(running_ == true);
    //通知start 函数 threadFunc 开始运行
    latch_.countDown();
    LogFile output(basename_, rollSize_, false);
    BufferPtr newBuffer1(new Buffer);
    BufferPtr newBuffer2(new Buffer);
    BufferVector buffersToWrite;
    //set size of buffersToWrite to 16
    buffersToWrite.reserve(16);
    while (running_) {
        assert(newBuffer1 && newBuffer1->length() == 0);
        assert(newBuffer2 && newBuffer2->length() == 0);
        assert(buffersToWrite.empty());
        {
            //保护buffers_ buffersToWrite_ 等
            muduo::MutexLockGuard lock(mutex_);
            //if buffers_ is empty ,wait for flushInterval_
            if (buffers_.empty()) {
                cond_.waitForSeconds(flushInterval_);
            }
            //append currentBuffer_ to buffers_
            buffers_.push_back(currentBuffer_.release());
            //replace currentBuffer with newBuffer1
            currentBuffer_ = boost::ptr_container::move(newBuffer1);
            //swap buffers_ with buffersToWrite
            buffersToWrite.swap(buffers_);
            //if nextBuffer_ is nullptr, replace it with newBuffer2
            if (!nextBuffer_) {
                nextBuffer_ = boost::ptr_container::move(newBuffer2);
            }
        }
        assert(!buffersToWrite.empty());
        if (buffersToWrite.size() > 25) {
            char buf[256];
            snprintf(buf, sizeof buf, "Dropped log message at %s, %zd larger buffers \n",
                     Timestamp::now().toFormattedString().c_str(), buffersToWrite.size() - 2);
            fputs(buf, stderr);
            output.append(buf, static_cast<int>(strlen(buf)));
            //erase begin +2 to end of the vector
            //保留两个元素用于 后面初始化newBuffer1 和 newBuffer2
            buffersToWrite.erase(buffersToWrite.begin() + 2, buffersToWrite.end());
        }
        for (size_t i = 0; i < buffersToWrite.size(); i++) {
            output.append(buffersToWrite[i].data(), buffersToWrite[i].length());
        }
        //resize buffersToWrite
        //only if size buffersToWrite is bigger than 2 that newBuffer1 and newBuffer2 may equal to nullptr
        if (buffersToWrite.size() > 2) {
            buffersToWrite.resize(2);
        }
        //重新设置newBuffer1
        if (!newBuffer1) {
            assert(!buffersToWrite.empty());
            newBuffer1 = buffersToWrite.pop_back();
            newBuffer1->reset();
        }
        //重新设置newBuffer2
        if (!newBuffer2) {
            assert(!buffersToWrite.empty());
            newBuffer2 = buffersToWrite.pop_back();
            newBuffer2->reset();
        }
        //clear write buffer
        buffersToWrite.clear();
        //flush ouput flow
        output.flush();
    }
    //exit thread func, flush again
    output.flush();
}