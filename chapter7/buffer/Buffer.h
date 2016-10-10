#ifndef MUDUO_NET_BUFFER_H
#define MUDUO_NET_BUFFER_H

#include "base/copyable.h"
#include "base/StringPiece.h"
#include "base/Types.h"

#include "Endian.h"

#include <algorithm>
#include <vector>

#include <assert.h>
#include <string.h>
//#include <unistd.h>  // ssize_t

namespace muduo {
    namespace net {

/// A buffer class modeled after org.jboss.netty.buffer.ChannelBuffer
///
/// @code
/// +-------------------+------------------+------------------+
/// | prependable bytes |  readable bytes  |  writable bytes  |
/// |                   |     (CONTENT)    |                  |
/// +-------------------+------------------+------------------+
/// |                   |                  |                  |
/// 0      <=      readerIndex   <=   writerIndex    <=     size
/// @endcode
        class Buffer : public muduo::copyable {
        public:
            static const size_t kCheapPrepend = 8;
            static const size_t kInitialSize = 1024;

            explicit Buffer(size_t initialSize = kInitialSize)
                    : buffer_(kCheapPrepend + initialSize),
                      readerIndex_(kCheapPrepend),
                      writerIndex_(kCheapPrepend) {
                assert(readableBytes() == 0);
                assert(writableBytes() == initialSize);
                assert(prependableBytes() == kCheapPrepend);
            }

            //swap 移动语义
            void swap(Buffer &&rhs) {
                std::vector<char> buf = std::move(rhs.buffer_);
                std::swap(rhs.readerIndex_, readerIndex_);
                std::swap(rhs.writerIndex_, writerIndex_);
                buffer_ = std::move(buf);
                rhs.buffer_ = std::move(buf);
            }


            // implicit copy-ctor, move-ctor, dtor and assignment are fine
            // NOTE: implicit move-ctor is added in g++ 4.6

            void swap(Buffer &rhs) {
                Buffer temp(rhs);
                buffer_.swap(rhs.buffer_);
                std::swap(readerIndex_, rhs.readerIndex_);
                std::swap(writerIndex_, rhs.writerIndex_);
            }

            //可读的buffer 大小
            size_t readableBytes() const { return writerIndex_ - readerIndex_; }

            //可写的buffer 大小
            size_t writableBytes() const { return buffer_.size() - writerIndex_; }

            //可以用于prepend的空间大小
            size_t prependableBytes() const { return readerIndex_; }

            //获取readerIndex_ 所在的指针
            const char *peek() const { return begin() + readerIndex_; }

            const char *findCRLF() const {
                // FIXME: replace with memmem()?
                //在范围 A 中查找第一个与范围 B 等价的子范围的位置,没有找到返回beginWrite()
                const char *crlf = std::search(peek(), beginWrite(), kCRLF, kCRLF + 2);
                return crlf == beginWrite() ? NULL : crlf;
            }

            const char *findCRLF(const char *start) const {
                assert(peek() <= start);
                assert(start <= beginWrite());
                // FIXME: replace with memmem()?
                const char *crlf = std::search(start, beginWrite(), kCRLF, kCRLF + 2);
                return crlf == beginWrite() ? NULL : crlf;
            }

            const char *findEOL() const {
                //在参数 str 所指向的字符串的前 n 个字节中搜索第一次出现字符 c
                //该函数返回一个指向匹配字节的指针，如果在给定的内存区域未出现字符，则返回 NULL
                const void *eol = memchr(peek(), '\n', readableBytes());
                return static_cast<const char *>(eol);
            }

            const char *findEOL(const char *start) const {
                assert(peek() <= start);
                assert(start <= beginWrite());
                const void *eol = memchr(start, '\n', beginWrite() - start);
                return static_cast<const char *>(eol);
            }

            // retrieve returns void, to prevent
            // string str(retrieve(readableBytes()), readableBytes());
            // the evaluation of two functions are unspecified

            //往后移动readerIndex_
            void retrieve(size_t len) {
                assert(len <= readableBytes());
                if (len < readableBytes())
                {
                    readerIndex_ += len;
                }
                else
                {
                    retrieveAll();
                }
            }

            void retrieveUntil(const char *end) {
                assert(peek() <= end);
                assert(end <= beginWrite());
                retrieve(end - peek());
            }

            void retrieveInt64() {
                retrieve(sizeof(int64_t));
            }

            void retrieveInt32() {
                retrieve(sizeof(int32_t));
            }

            void retrieveInt16() {
                retrieve(sizeof(int16_t));
            }

            void retrieveInt8() {
                retrieve(sizeof(int8_t));
            }

            //reset readerIndex_ and writeIndex_
            void retrieveAll() {
                readerIndex_ = kCheapPrepend;
                writerIndex_ = kCheapPrepend;
            }

            string retrieveAllAsString() {
                return retrieveAsString(readableBytes());;
            }

            string retrieveAsString(size_t len) {
                assert(len <= readableBytes());
                string result(peek(), len);
                retrieve(len);
                return result;
            }

            StringPiece toStringPiece() const {
                return StringPiece(peek(), static_cast<int>(readableBytes()));
            }

            void append(const StringPiece &str) {
                append(str.data(), str.size());
            }

            void append(const char * /*restrict*/ data, size_t len) {
                ensureWritableBytes(len);
                std::copy(data, data + len, beginWrite());
                hasWritten(len);
            }

            void append(const void * /*restrict*/ data, size_t len) {
                append(static_cast<const char *>(data), len);
            }

            void ensureWritableBytes(size_t len) {
                //如果可写的空间小于len
                if (writableBytes() < len)
                {
                    // 开辟空间
                    makeSpace(len);
                }
                assert(writableBytes() >= len);
            }

            char *beginWrite() { return begin() + writerIndex_; }

            const char *beginWrite() const { return begin() + writerIndex_; }

            //set writerIndex_ forward len pos
            void hasWritten(size_t len) {
                assert(len <= writableBytes());
                writerIndex_ += len;
            }

            //set writerIndex_ back len pos
            void unwrite(size_t len) {
                assert(len <= readableBytes());
                writerIndex_ -= len;
            }

            ///
            /// Append int64_t using network endian
            ///
            void appendInt64(int64_t x) {
                int64_t be64 = sockets::hostToNetwork64(x);
                append(&be64, sizeof be64);
            }

            ///
            /// Append int32_t using network endian
            ///
            void appendInt32(int32_t x) {
                int32_t be32 = sockets::hostToNetwork32(x);
                append(&be32, sizeof be32);
            }

            void appendInt16(int16_t x) {
                int16_t be16 = sockets::hostToNetwork16(x);
                append(&be16, sizeof be16);
            }

            void appendInt8(int8_t x) {
                append(&x, sizeof x);
            }

            ///
            /// Read int64_t from network endian
            ///
            /// Require: buf->readableBytes() >= sizeof(int32_t)
            int64_t readInt64() {
                int64_t result = peekInt64();
                retrieveInt64();
                return result;
            }

            ///
            /// Read int32_t from network endian
            ///
            /// Require: buf->readableBytes() >= sizeof(int32_t)
            int32_t readInt32() {
                int32_t result = peekInt32();
                retrieveInt32();
                return result;
            }

            int16_t readInt16() {
                // 首先取出数据
                int16_t result = peekInt16();
                // 然后重新设置readerIndex_和writerIndex_
                retrieveInt16();
                return result;
            }

            int8_t readInt8() {
                int8_t result = peekInt8();
                retrieveInt8();
                return result;
            }

            ///
            /// Peek int64_t from network endian
            ///
            /// Require: buf->readableBytes() >= sizeof(int64_t)
            int64_t peekInt64() const {
                assert(readableBytes() >= sizeof(int64_t));
                int64_t be64 = 0;
                ::memcpy(&be64, peek(), sizeof be64);
                return sockets::networkToHost64(be64);
            }

            ///
            /// Peek int32_t from network endian
            ///
            /// Require: buf->readableBytes() >= sizeof(int32_t)
            int32_t peekInt32() const {
                assert(readableBytes() >= sizeof(int32_t));
                int32_t be32 = 0;
                ::memcpy(&be32, peek(), sizeof be32);
                return sockets::networkToHost32(be32);
            }

            int16_t peekInt16() const {
                assert(readableBytes() >= sizeof(int16_t));
                int16_t be16 = 0;
                ::memcpy(&be16, peek(), sizeof be16);
                return sockets::networkToHost16(be16);
            }

            int8_t peekInt8() const {
                assert(readableBytes() >= sizeof(int8_t));
                int8_t x = *peek();
                return x;
            }

            ///
            /// Prepend int64_t using network endian
            ///
            void prependInt64(int64_t x) {
                int64_t be64 = sockets::hostToNetwork64(x);
                prepend(&be64, sizeof be64);
            }

            ///
            /// Prepend int32_t using network endian
            ///
            void prependInt32(int32_t x) {
                int32_t be32 = sockets::hostToNetwork32(x);
                prepend(&be32, sizeof be32);
            }

            void prependInt16(int16_t x) {
                int16_t be16 = sockets::hostToNetwork16(x);
                prepend(&be16, sizeof be16);
            }

            void prependInt8(int8_t x) {
                prepend(&x, sizeof x);
            }

            void prepend(const void * /*restrict*/ data, size_t len) {
                assert(len <= prependableBytes());
                //将读指针前移len位置
                readerIndex_ -= len;
                const char *d = static_cast<const char *>(data);
                //将数据复制进入prepend位置
                std::copy(d, d + len, begin() + readerIndex_);
                //此时无需变动readerIndex_位置，因为新增的内容需要转发到网络
            }

            void shrink(size_t reserve) {
                // FIXME: use vector::shrink_to_fit() in C++ 11 if possible.
                // shrink to fit make size equal to capacity
                Buffer other;
                other.ensureWritableBytes(readableBytes() + reserve);
                other.append(toStringPiece());
                swap(std::move(other));
            }

            size_t internalCapacity() const {
                return buffer_.capacity();
            }

            /// Read data directly into buffer.
            ///
            /// It may implement with readv(2)
            /// @return result of read(2), @c errno is saved
            ssize_t readFd(int fd, int *savedErrno);

        private:

            char *begin() { return &*buffer_.begin(); }

            const char *begin() const { return &*buffer_.begin(); }

            void makeSpace(size_t len) {
                if (writableBytes() + prependableBytes() < len + kCheapPrepend)
                {
                    // FIXME: move readable data
                    buffer_.resize(writerIndex_ + len);
                }
                else
                {
                    // move readable data to the front, make space inside buffer
                    assert(kCheapPrepend < readerIndex_);
                    size_t readable = readableBytes();
                    //将[begin()+readeIndex_,begin()+writerIndex_)中间的数据写入从kCheapPrepend对应的位置
                    std::copy(begin() + readerIndex_,
                              begin() + writerIndex_,
                              begin() + kCheapPrepend);
                    readerIndex_ = kCheapPrepend;
                    //reset writerIndex_
                    writerIndex_ = readerIndex_ + readable;
                    assert(readable == readableBytes());
                }
            }


        private:
            std::vector<char> buffer_;
            size_t readerIndex_;
            size_t writerIndex_;

            //定义的分隔符为 "\r\n"
            static const char kCRLF[];
        };

    }
}

#endif  // MUDUO_NET_BUFFER_H