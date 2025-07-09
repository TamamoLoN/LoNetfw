cmake_minimum_required(VERSION 3.1)

add_library(util SHARED
            ${CMAKE_SOURCE_DIR}/util/util.cpp
            )

target_include_directories(util PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})