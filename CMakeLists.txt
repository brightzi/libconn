cmake_minimum_required(VERSION 3.0)

project(LIBCONN)

# Set the build type to Debug by default
set(CMAKE_BUILD_TYPE Debug)

# Set output directories for libraries
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Specify include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src/dispatcher
    ${CMAKE_SOURCE_DIR}/src/http
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/ssl
)
set(CONN_LIB "conn")

# Add necessary subdirectories
add_subdirectory(src)
add_subdirectory(bin)
