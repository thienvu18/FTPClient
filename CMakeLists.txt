cmake_minimum_required(VERSION 3.10)
project(FTPClient)

set(CMAKE_CXX_STANDARD 11)
set(SOURCE lib/TCPClient.cpp FTPClient.cpp main.cpp)

add_executable(FTPClient ${SOURCE})