cmake_minimum_required(VERSION 3.5)
project(protobuf)
include_directories(/home/zhouyang/Documents/muduo/include)
link_directories(/home/zhouyang/Documents/muduo/lib)
find_package(Threads)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(source_code_1 descriptor_test.cpp query.pb.h query.pb.cc)
add_executable(descriptor_test ${source_code_1})
target_link_libraries(descriptor_test protobuf)
set(source_code_2 codec_test.cpp codec.h query.pb.h query.pb.cc)
add_executable(codec_test ${source_code_2})
target_link_libraries(codec_test protobuf)




