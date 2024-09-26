cmake_minimum_required(VERSION 3.0)

project(LIBCONN)
set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)


set(INCLUDES ./include)
set(DISP_INCLUDES ./src/dispatcher)
include_directories(${INCLUDES})
include_directories(${DISP_INCLUDES})


set(CONN_LIB "conn")

add_subdirectory(src)
add_subdirectory(bin)



