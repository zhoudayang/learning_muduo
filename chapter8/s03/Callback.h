//
// Created by zhouyang on 16-9-28.
//

#ifndef S02_CALLBACK_H
#define S02_CALLBACK_H

#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include "base/Timestamp.h"

namespace muduo {

    //Timer Callback function
    typedef boost::function<void()> TimerCallback;

}
#endif //S02_CALLBACK_H
