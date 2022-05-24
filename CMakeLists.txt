cmake_minimum_required(VERSION 3.5)

project(cat LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB SRCS *.cpp *.h)

add_library(cat SHARED ${SRCS})

add_executable(app main.cpp test.h)

target_link_libraries(app cat)
