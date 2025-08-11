cmake_minimum_required(VERSION 3.1)

link_directories(${CMAKE_SOURCE_DIR}/lib
                ${CMAKE_SOURCE_DIR}/third_party/yaml-cpp
                 )

include_directories(${CMAKE_SOURCE_DIR}
                    ${CMAKE_SOURCE_DIR}/src
                    ${CMAKE_SOURCE_DIR}/third_party
                    )

file(GLOB TIMER_SRCS ${CMAKE_SOURCE_DIR}/timer/*.cpp)

add_library(timer SHARED
            ${TIMER_SRCS}
            )

target_link_libraries(timer PRIVATE util thread)
target_include_directories(timer PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/util)