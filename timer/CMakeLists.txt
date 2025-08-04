cmake_minimum_required(VERSION 3.1)

include_directories(${CMAKE_SOURCE_DIR})

file(GLOB TIMER_SRCS ${CMAKE_SOURCE_DIR}/timer/*.cpp)

add_library(timer SHARED
            ${TIMER_SRCS}
            )

target_link_libraries(timer PRIVATE util thread)
target_include_directories(timer PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/util)