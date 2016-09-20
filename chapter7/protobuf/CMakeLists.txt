cmake_minimum_required(VERSION 2.8)
project(protobuf)
include_directories(/home/zhouyang/Documents/muduo/include)
link_directories(/home/zhouyang/Documents/muduo/lib)
include(FindProtobuf)
find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIR})
find_package(Threads)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(source_code_1 descriptor_test.cpp query.pb.h query.pb.cc)
add_executable(descriptor_test ${source_code_1})
target_link_libraries(descriptor_test ${PROTOBUF_LIBRARY})
set(source_code_2 codec_test.cpp codec.h query.pb.h query.pb.cc)
add_executable(codec_test ${source_code_2})
target_link_libraries(codec_test protobuf z)




