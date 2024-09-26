cmake_minimum_required(VERSION 3.0)

project(LIBCONN)

set(INCLUDES ./include)
include_directories(${INCLUDES})

set(CMAKE_BUILD_TYPE Debug)
set(LIBCONN_LIBRARIES  "${CMAKE_BINARY_DIR}/libconn.a")
add_subdirectory(src)
add_subdirectory(bin)



