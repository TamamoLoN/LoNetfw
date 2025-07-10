cmake_minimum_required(VERSION 3.1)

file(GLOB UTIL_SRCS ${CMAKE_SOURCE_DIR}/util/*.cpp)

add_library(util SHARED
            ${UTIL_SRCS}
            )

target_include_directories(util PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})