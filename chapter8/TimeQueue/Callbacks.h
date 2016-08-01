//
// Created by zhouyang on 16/8/1.
//

#ifndef CALLBACKS_H
#define CALLBACKS_H

#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>

#include "datetime/Timestamp.h"

namespace muduo {
    typedef boost::function<void()> TimerCallback;
}
#endif
