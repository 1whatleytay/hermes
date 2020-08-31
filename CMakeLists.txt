cmake_minimum_required(VERSION 3.17)
project(hermes)

set(CMAKE_CXX_STANDARD 14)

add_library(hermes
    include/hermes/error.h
    include/hermes/state.h
    include/hermes/node.h

    src/error.cpp
    src/state.cpp
    src/node.cpp)

target_include_directories(hermes PUBLIC include)
