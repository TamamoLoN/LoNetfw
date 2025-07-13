cmake_minimum_required(VERSION 3.1)

include_directories(${CMAKE_SOURCE_DIR})

file(GLOB THREAD_SRCS ${CMAKE_SOURCE_DIR}/thread/*.cpp)

add_library(thread SHARED
            ${THREAD_SRCS}
            )

target_link_libraries(thread PRIVATE util log config pthread)
target_include_directories(thread PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/util)