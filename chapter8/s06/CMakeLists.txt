cmake_minimum_required(VERSION 3.5)
project(s06)
find_package(Threads)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
add_subdirectory(base)
set(LIBRARY_SOURCE
        Channel.cpp
        Callback.h
        EventLoop.cpp
        Poller.cpp
        Timer.cpp
        TimerQueue.cpp
        TimerId.h
        EventLoopThread.cpp
        SocketsOps.cpp
        InetAddress.cpp
        Sockets.cpp
        Acceptor.cpp
        TcpConnection.cpp
        TcpServer.cpp
        )
add_library(event ${LIBRARY_SOURCE})
target_link_libraries(event base ${CMAKE_THREAD_LIBS_INIT})
add_executable(test6 test6.cpp)
target_link_libraries(test6 event)
add_executable(test7 test7.cpp)
target_link_libraries(test7 event)
add_executable(test8 test8.cpp)
target_link_libraries(test8 event)
