cmake_minimum_required(VERSION 3.5)

include(GenerateExportHeader)

project(cat LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB SRCS *.cpp *.h)

add_library(cat SHARED ${SRCS})
generate_export_header(cat EXPORT_FILE_NAME ${CMAKE_BINARY_DIR}/exports/cat_export.h)

include_directories(${CMAKE_BINARY_DIR}/exports/)

add_executable(app main.cpp test.h)

target_link_libraries(app cat)
