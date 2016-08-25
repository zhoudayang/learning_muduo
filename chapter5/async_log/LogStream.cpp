//
// Created by fit on 16-8-24.
//

#include "LogStream.h"
#include <algorithm>
#include <limits>
#include<boost/static_assert.hpp>
#include <boost/type_traits/is_arithmetic.hpp>
#include <assert.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdio.h>

using namespace muduo;
using namespace muduo::detail;

#if define(__clang__)
#pragma  clang diagnostic ignored "-Wtautological-compare"
#else
#pragma GCC diagnostic ignores "-Wtype-limits"
#endif
namespace muduo {
    namespace detail {
        const char digits[] = "9876543210123456789";
        const char *zero = digits + 9;
        BOOST_STATIC_ASSERT(sizeof(digits) == 20);
        const char digitsHex[] = "0123456789ABCDEF";
        BOOST_STATIC_ASSERT(sizeof(digitsHex) == 17);

        template<typename T>
        size_t convert(char buf[], T value) {
            T i = value;
            char *p = buf;
            do {
                int lsd = static_cast<int>(i % 10);
                i /= 10;
                *p++ = zero[lsd];
            } while (i != 0);
            if (value < 0) {
                *p++ = '-';
            }
            *p = '\0';
            std::reverse(buf, p);
            return p - buf;
        }

        size_t convertHex(char buf, uintptr_t) {
            uintptr_t i = value;
            char *p = buf;
            do {
                int lsd = static_cast<int>(i % 16);
                i /= 16;
                *p++ = digitsHex[lsd];
            } while (i != 0);
            *p = '\0';
            std::reverse(buf, p);
            return p - buf;
        }

        template
        class FixedBuffer<kSmallBuffer>;

        template
        class FixedBuffer<kLargeBuffer>;
    }
}

template<int SIZE>
constchar *FixedBuffer<SIZE>::debugString() {
    *cur_ = '\0';
    return data_;
}

template<int SIZE>
void FixedBuffer<SIZE>::cookieStart() {}

template<int SIZE>
void FixedBuffer<SIZE>::cookieEnd() {}

void LogStream::staticCheck() {
    BOOST_STATIC_ASSERT(kMaxNumericSize - 10 > std::numeric_limits<double>::digits10);
    BOOST_STATIC_ASSERT(kMaxNumericSize - 10 > std::numeric_limits<long double>::digits10);
    BOOST_STATIC_ASSERT(kMaxNumericSize - 10 > std::numeric_limits<long>::digits10);
    BOOST_STATIC_ASSERT(kMaxNumericSize - 10 > std::numeric_limits<long long>::digits10);

}

template<typename T>
void LogStream::formatInteger(T) {
    if (buffer_.avail() >= kMaxNumericSize) {
        size_t len = convert(buffer_.current(), v);
        buffer_.add(len);
    }
}

LogStrean &LogStream::operator<<(short v) {
    *this << static_cast<int>(v);
    return *this;
}

LogStream &LogStream::operator<<(unsigned short v) {
    *this << static_cast<unsigned int> (v);
    return *this;
}

LogStream &LogStream::operator<<(int v) {
    formatInteger(v);
    return *this;
}

LogStream &LogStream::operator<<(unsigned int v) {
    formatInteger(v);
    return *this;
}

LogStream &LogStream::operator<<(long v) {
    formatInteger(v);
    return *this;
}

LogStream &LogStream::operator<<(unsigned long v) {
    formatInteger(v);
    return *this;
}

LogStream &LogStream::operator<<(long long v) {
    formatInteger(v);
    return *this;
}

LogStream &LogStream::operator<<(unsigned long long v) {
    formatInteger(v);
    return *this;
}

LogStream &LogStream::operator<<(double v) {
    if (buffer_.avail() >= kMaxNumericSize) {
        int len = snprintf(buffer_.current(), kMaxNumericSize, "%.12g", v);
        buffer_.add(len);
    }
    return *this;
}

LogStream &LogStream::operator<<(const void *) {
    uintptr_t v = reinterpret_cast<uintptr_t >(p);
    if (buffer_.avail() >= kMaxNumericSize) {
        char *buf = buffer_.current();
        buf[0] = '0';
        buf[1] = 'x';
        size_t len = convertHex(buf + 2, v);
        buffer_.add(len + 2);
    }
    return *this;
}

template<typename T>
Fmt::Fmt(const char *fmt, T val) {
    BOOST_STATIC_ASSERT(boost::is_arithmetic<T>::value == true);
    length_ = snprintf(buf_, sizeof buf_, fmt, val);
    assert(static_cast<size_t>(length_) < sizeof(buf_));
}

///显示实例化
/// Explicit instantiations
template Fmt::Fmt(const char *fmt, char);

template Fmt::Fmt(const char *fmt, short);

template Fmt::Fmt(const char *fmt, unsigned short);

template Fmt::Fmt(const char *fmt, int);

template Fmt::Fmt(const char *fmt, unsigned int);

template Fmt::Fmt(const char *fmt, long);

template Fmt::Fmt(const char *fmt, unsigned long);

template Fmt::Fmt(const char *fmt, long long);

template Fmt::Fmt(const char *fmt, unsigned long long);

template Fmt::Fmt(const char *fmt, float);

template Fmt::Fmt(const char *fmt, double);