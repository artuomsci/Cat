cmake_minimum_required(VERSION 3.5)

include(GenerateExportHeader)

project(cat LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB INCLUDE include/*.h test/*.h)
file(GLOB SRCS src/*.cpp src/*.h)

add_library(cat SHARED ${INCLUDE} ${SRCS})

set(PROJECT_VERSION 0.0.1)
set_target_properties(cat PROPERTIES VERSION ${PROJECT_VERSION})

include_directories(include)
include_directories(${CMAKE_BINARY_DIR}/exports/)

generate_export_header(cat EXPORT_FILE_NAME ${CMAKE_BINARY_DIR}/exports/cat_export.h)
