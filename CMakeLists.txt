cmake_minimum_required(VERSION 3.10)

# Sets project name and version
project(LiteTorrent VERSION 0.1
DESCRIPTION "LiteTorrent is a peer-to-peer file sharing network")

# Specifies the C++ standard to C++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(OPENSSL_DIR "/usr/local" CACHE STRING "The openssl 3.0.3 directory (defaults to /usr/local)")
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# link_directories(${OPENSSL_DIR}/lib64)
# include_directories(include ${OPENSSL_DIR}/include)

add_subdirectory(src)
