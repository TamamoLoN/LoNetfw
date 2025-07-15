cmake_minimum_required(VERSION 3.1)

include_directories(${CMAKE_SOURCE_DIR})

file(GLOB LOG_SRCS ${CMAKE_SOURCE_DIR}/log/*.cpp)

add_library(log SHARED
            ${LOG_SRCS}
            )

target_link_libraries(log PRIVATE util fiber thread)
target_include_directories(log PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/util)