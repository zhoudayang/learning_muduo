#ifndef TYPES_H
#define TYPES_H

//only for gcc
//don't support gcc
#include <stdint.h>
#include <ext/vstring.h>
#include <ext/vstring_fwd.h>
#include <assert.h>

//pass
namespace muduo {
    typedef __gnu_cxx::__sso_string string;

    // 只能up_cast 不能down_cast
    template<typename To, typename From>
    inline To implicit_cast(From const &f) {
        return f;
    };

    template<typename To, typename From>
    inline To down_cast(From *f) {
        // Ensures that To is a sub-type of From *.  This test is here only
        // for compile-time type checking, and has no overhead in an
        // optimized build at run-time, as it will be optimized away
        // completely.
        // 编译期检查
        if (false) {
            // implicit_cast<up_class,down_class> only support upcast
            // if use as implicit_cast<down_class,up_class>, will get compile error
            implicit_cast<From *, To>(0);
        }
        return static_cast<To>(f);
    };
}


#endif

