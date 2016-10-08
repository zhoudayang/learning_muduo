//
// Created by zhouyang on 16-9-27.
//

#include "Channel.h"
#include "EventLoop.h"
#include "base/Logging.h"

#include <sstream>
#include <poll.h>

using namespace muduo;

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = POLLIN|POLLPRI;
const int Channel::kWriteEvent = POLLOUT;


/*
              POLLIN There is data to read. 有数据可读

              POLLPRI　　有紧急数据可读
                     There is urgent data to read (e.g., out-of-band data on
                     TCP socket; pseudoterminal master in packet mode has
                     seen state change in slave).

              POLLOUT　　现在可以写数据
                     Writing is now possible, though a write larger that the
                     available space in a socket or pipe will still block
                     (unless O_NONBLOCK is set).

              POLLRDHUP (since Linux 2.6.17)　对方关闭了连接，不再进一步写入数据
                     Stream socket peer closed connection, or shut down
                     writing half of connection.  The _GNU_SOURCE feature
                     test macro must be defined (before including any header
                     files) in order to obtain this definition.

              POLLERR　发生了错误
                     Error condition (only returned in revents; ignored in
                     events).

              POLLHUP
              //stackoverflow
                A device has been disconnected,
                or a pipe or FIFO has been closed by the last process that had it open for writing.
                Once set, the hangup state of a FIFO shall persist until some process opens the FIFO for writing or until all read-only file descriptors for the FIFO are closed.
                This event and POLLOUT are mutually-exclusive; a stream can never be writable if a hangup has occurred. 
                However, this event and POLLIN, POLLRDNORM, POLLRDBAND, or POLLPRI are not mutually-exclusive. 
                This flag is only valid in the revents bitmask; it shall be ignored in the events member.

              //
                     Hang up (only returned in revents; ignored in events).
                     Note that when reading from a channel such as a pipe or
                     a stream socket, this event merely indicates that the
                     peer closed its end of the channel.  Subsequent reads
                     from the channel will return 0 (end of file) only after
                     all outstanding data in the channel has been consumed.

              POLLNVAL　／／错误的请求，fd　没有打开
                     Invalid request: fd not open (only returned in revents;
                     ignored in events).

       When compiling with _XOPEN_SOURCE defined, one also has the
       following, which convey no further information beyond the bits listed
       above:

              POLLRDNORM
                     Equivalent to POLLIN.

              POLLRDBAND
                     Priority band data can be read (generally unused on
                     Linux).

              POLLWRNORM
                     Equivalent to POLLOUT.

              POLLWRBAND
                     Priority data may be written.

 */


Channel::Channel(EventLoop *loop, int fdArg) :
    loop_(loop),
    fd_(fdArg),
    events_(0),
    revents_(0),
    index_(-1)
{

}

void Channel::update(){
    loop_->updateChannel(this);
}


void Channel::handleEvent(){
    //invalid file descriptor
    if(revents_ & POLLNVAL){
        LOG_WARN<<"Channle::handleEvent() POLLNVAL";
    }
    //poll error
    if(revents_ & (POLLERR|POLLNVAL)){
        if(errorCallback_){
            errorCallback_();
        }
    }
    //read event
    if(revents_ & (POLLIN|POLLPRI|POLLRDHUP)){
        if(readCallback_)
            readCallback_();
    }
    //now can write data, call write callback function
    if(revents_&POLLOUT){
        if(writeCallback_)
            writeCallback_();
    }
}