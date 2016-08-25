//
// Created by zhouyang on 16-8-25.
//

#ifndef ASYNC_LOG_FILEUTIL_H
#define ASYNC_LOG_FILEUTIL_H

namespace muduo{
    namespace FileUtil{
        class ReadSmallFile:boost::noncopyable{
        public:
            ReadSmallFile(StringArg filename);
            ~ReadSmallFile();
            template <typename String>
            int readToString(int maxSize,String *content,int64_t *fileSize,int64_t *modifyTime,int64_t * createTime);

            int readToBuffer(int *size);

            const char *buffer() const{
                return buf_;
            }

            static const int kBufferSize = 64*1024;
        private:
            int fd_;
            int err_;
            char buf_[kBufferSize];
        };

        template<typename String>
        int readFile(StringArg filename,int maxSize,String * content,int64_t * fileSize = NULL,int64_t * modifyTime = NULL,int64_t * createTime =NULL){
            ReadSmallFile file(filename);
            return file.readToString(maxsize,content,fileSize,modifyTime,createTime);
        }
        class AppendFile:boost::noncopyable{
        public:
            explicit AppendFile(StringArg filename);
            ~AppendFile();
            void append(const char * logline,const size_t len);
            void flush();
            size_t writtenBytes() const{
                return writtenBytes_;
            }
        private:
            size_t write(const char * logline,size_t len);
            FILE *fp_;
            char buffer_[64*1024];
            size_t writtenBytes_;
        };
    }


}


#endif //ASYNC_LOG_FILEUTIL_H
