#include "codec.h"

using namespace muduo;
using namespace muduo::net;
using namespace pubsub;

ParseResult pubsub::parseMessage(Buffer* buf,
                                 string* cmd,
                                 string* topic,
                                 string* content)
{
    ParseResult result = kError;
    //broadcast information
    //pub <topic>\r\n<content>\r\n
    const char* crlf = buf->findCRLF();
    if (crlf)
    {
        const char* space = std::find(buf->peek(), crlf, ' ');
        if (space != crlf)
        {
            //command
            cmd->assign(buf->peek(), space);
            //topic
            topic->assign(space+1, crlf);
            if (*cmd == "pub")
            {
                const char* start = crlf + 2;
                crlf = buf->findCRLF(start);
                if (crlf)
                {
                    //get content
                    content->assign(start, crlf);
                    buf->retrieveUntil(crlf+2);
                    result = kSuccess;
                }
                else
                {
                    result = kContinue;
                }
            }
            else
            {
                buf->retrieveUntil(crlf+2);
                result = kSuccess;
            }
        }
        else
        {
            result = kError;
        }
    }
        //find not crlf, continue
    else
    {
        result = kContinue;
    }
    return result;
}