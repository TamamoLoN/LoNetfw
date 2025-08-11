cmake_minimum_required(VERSION 3.1)

link_directories(${CMAKE_SOURCE_DIR}/lib
                ${CMAKE_SOURCE_DIR}/third_party/yaml-cpp
                 )

include_directories(${CMAKE_SOURCE_DIR}
                    ${CMAKE_SOURCE_DIR}/src
                    ${CMAKE_SOURCE_DIR}/third_party
                    )

file(GLOB THREAD_SRCS ${CMAKE_SOURCE_DIR}/thread/*.cpp)

add_library(thread SHARED
            ${THREAD_SRCS}
            )

target_link_libraries(thread PRIVATE util pthread)
target_include_directories(thread PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/util)