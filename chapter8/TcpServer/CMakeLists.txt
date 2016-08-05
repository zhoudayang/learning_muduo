cmake_minimum_required(VERSION 2.8)
project(TcpServer)
find_package(Threads)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(lib_src
        datetime/Timestamp.cc
        Atomic.h
        Callbacks.h
        Timer.cpp
        TimerId.h
        Thread.cpp
        EventLoop.cpp
        Poller.cpp
        Channel.cpp
        TimerQueue.cpp
        EventLoopThread.cpp
        Socket.cpp
        SocketsOps.cpp
        Acceptor.cpp
        InetAddress.cpp
        TcpConnection.cpp
        TcpServer.cpp
        )
#编译生成动态链接库
add_library(event SHARED ${lib_src})

add_executable( test8 test.cpp)
target_link_libraries(test8 event ${CMAKE_THREAD_LIBS_INIT})