// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//

#include "Buffer.h"

#include "SocketsOps.h"

#include <errno.h>
#include <sys/uio.h>

using namespace muduo;
using namespace muduo::net;

const char Buffer::kCRLF[] = "\r\n";

const size_t Buffer::kCheapPrepend;
const size_t Buffer::kInitialSize;
//在非阻塞网络编程中，如何设计并使用缓冲区？一方面我们希望减少系统调用，一次
//读的数据越多越划算，那似乎应该准备一个大的缓冲区．另一方面希望减少内存调用，
//如果有10000个并发连接，每个链接一建立就分配50kB的读写缓冲区的话，将占用1GB
//内存，而大多数时候这些缓冲区的使用率很低　
/*
 * muduo使用readv结合栈上空间解决这个问题.在栈上准备一个65536字节的extrabuf,
 * 然后利用readv()来读数据，iovec有两块，第一块指向muduo Buffer中的writable
 * 字节，另一块指向栈上的extrabuf.这样如果读入的数据不多，那么全部都读到Buffer中
 * 去了；如果长度超过Buffer的writable 字节数，那么就会读到栈上的extrabuf中，然后
 * 程序再将extrabuf中的数据append到Buffer中．
 */
ssize_t Buffer::readFd(int fd, int* savedErrno)
{
    // saved an ioctl()/FIONREAD call to tell how much to read
    // 临时栈上空间
    char extrabuf[65536];
    struct iovec vec[2];
    const size_t writable = writableBytes();
    vec[0].iov_base = begin()+writerIndex_;
    vec[0].iov_len = writable;
    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof extrabuf;
    // when there is enough space in this buffer, don't read into extrabuf.
    // when extrabuf is used, we read 128k-1 bytes at most.
    const int iovcnt = (writable < sizeof extrabuf) ? 2 : 1;
    const ssize_t n = sockets::readv(fd, vec, iovcnt);
    if (n < 0)
    {
        *savedErrno = errno;
    }
    else if (implicit_cast<size_t>(n) <= writable)
    {
        writerIndex_ += n;
    }
    else
    {
        writerIndex_ = buffer_.size();
        //将extrabuf中的内容写入buffer_
        append(extrabuf, n - writable);
    }
    return n;
}