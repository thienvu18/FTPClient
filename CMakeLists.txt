cmake_minimum_required(VERSION 3.10)
project(FTPClient)

find_package( Threads REQUIRED )

set(CMAKE_CXX_STANDARD 17)
set(LIB lib/TCPClient.cpp lib/TCPServer.cpp)
set(SOURCE FTPClient.cpp main.cpp)

add_executable(FTPClient ${LIB} ${SOURCE})
target_link_libraries(FTPClient pthread)