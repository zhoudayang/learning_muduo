cmake_minimum_required(VERSION 3.5)
project(s02)
find_package(Threads)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(SOURCE_FILES
        Types.h
        Mutex.h
        Condition.cpp
        copyable.h
        BoundedBlockingQueue.h
        BlockingQueue.h
        CountDownLatch.cpp
        StringPiece.h
        Timestamp.cpp
        TimeZone.cpp
        Date.cpp
        CurrentThread.h
        Thread.cpp
        ProcessInfo.cpp
        LogFile.cpp
        Logging.cpp
        LogStream.cpp
        AsyncLogging.cpp
        FileUtil.cpp
        Exception.cpp
        Atomic.h
        ThreadPool.cpp
        )
add_library(base ${SOURCE_FILES})
target_link_libraries(base ${CMAKE_THREAD_LIBS_INIT})
