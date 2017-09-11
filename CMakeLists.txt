cmake_minimum_required(VERSION 3.7)
project(schedule)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp DataItem.h DataItem.cpp Task.h Task.cpp)
add_executable(schedule ${SOURCE_FILES})