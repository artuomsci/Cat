cmake_minimum_required(VERSION 3.5)

project(cat LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(cat SHARED cat.h cat.cpp str_utils.h str_utils.cpp)

add_executable(app main.cpp test.h)

target_link_libraries(app cat)
