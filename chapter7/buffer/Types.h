//
// Created by zhouyang on 16-9-13.
//

#ifndef PROJECT_TYPES_H
#define PROJECT_TYPES_H
#include <stdint.h>
#include <string>

#ifndef NDEBUG
#include <assert.h>
#endif

namespace muduo{
    using std::string;
    //向上转型　
    template <typename To,typename From>
    inline To implicit_cast(From const &f){
        return f;
    };

    //向下转型
    template <typename To,typename From>
    inline To down_cast(From * f){
        if(false){
            implicit_cast<From *,To>(0);
        }
        return static_cast<To>(f);
    };
}


#endif //PROJECT_TYPES_H
