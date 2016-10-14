cmake_minimum_required(VERSION 3.5)
project(s11)
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
        Buffer.cpp
        EventLoopThreadPool.cpp
        Connector.cpp
        )
add_library(event ${LIBRARY_SOURCE})
target_link_libraries(event base ${CMAKE_THREAD_LIBS_INIT})

add_executable(test8 test8.cpp)
target_link_libraries(test8 event)

add_executable(test9 test9.cpp)
target_link_libraries(test9 event)

add_executable(test10 test10.cpp)
target_link_libraries(test10 event)

add_executable(error error.cpp)
target_link_libraries(error event)


add_executable(test11 test11.cpp)
target_link_libraries(test11 event)
