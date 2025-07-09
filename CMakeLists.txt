cmake_minimum_required(VERSION 3.1)
set(PROJECT_NAME LoNetfw)
project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

set(CMAKE_BUILD_TYPE Debug)

add_definitions(-g -LINUX_X86)

# set(CMAKE_CXX_COMPILER "aarch64-linux-gnu-g++-8")
set(CMAKE_CXX_COMPILER "g++")

add_subdirectory(util)
add_subdirectory(log)

link_directories(${CMAKE_SOURCE_DIR}/lib
                 )

include_directories(${CMAKE_SOURCE_DIR}
                    ${CMAKE_SOURCE_DIR}/src
                    )

add_executable(${PROJECT_NAME}
               ${CMAKE_SOURCE_DIR}/src/main.cpp
               )

target_link_libraries(${PROJECT_NAME} PRIVATE util log) 