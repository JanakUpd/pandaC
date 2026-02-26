cmake_minimum_required(VERSION 4.0)
project(pandaC)

set(CMAKE_CXX_STANDARD 20)

add_executable(pandaC main.cpp
        libraries/pct.cpp)
