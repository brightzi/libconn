cmake_minimum_required(VERSION 3.0)

project(LIBCONN)
set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)


set(INCLUDES ./include)
set(DISPATCHER_INCLUDES ./src/dispatcher)
set(HTTP_INCLUDES ./src/http)
set(SRC_INCLUDES ./src)
include_directories(${INCLUDES})
include_directories(${DISPATCHER_INCLUDES})
include_directories(${HTTP_INCLUDES})
include_directories(${SRC_INCLUDES})


set(CONN_LIB "conn")

add_subdirectory(src)
add_subdirectory(bin)



