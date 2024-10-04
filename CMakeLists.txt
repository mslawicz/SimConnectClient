cmake_minimum_required(VERSION 3.20.0)
project(SimConnectClient VERSION 0.0.1 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)

add_executable(SimConnectClient 
    src/SimConnectClient.cpp
)

#set_property(TARGET SimConnectClient PROPERTY CXX_STANDARD 17)
