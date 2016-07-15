cmake_minimum_required(VERSION 2.8)
project(/home/box/final/final)

set(SOURCES HttpServer.cpp)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O0 -pthread -DUNIX")

add_executable(${PROJECT_NAME} ${SOURCES})
