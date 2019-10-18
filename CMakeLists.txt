cmake_minimum_required(VERSION 2.8)
project(tmpmanager)

SET(CMAKE_CXX_STANDARD 11)

include_directories(third/minilogger)
include_directories(src)

add_executable(tmpmanager 
    app/tmpmanager/main.cpp
    )
