cmake_minimum_required(VERSION 3.1)

link_directories(${CMAKE_SOURCE_DIR}/lib
                 )

include_directories(${CMAKE_SOURCE_DIR}
                    ${CMAKE_SOURCE_DIR}/src
                    ${YAML_INCLUDE_PATH}
                    )

file(GLOB FIBER_SRCS ${CMAKE_SOURCE_DIR}/fiber/*.cpp)

add_library(fiber SHARED
            ${FIBER_SRCS}
            )

target_link_libraries(fiber PRIVATE util)
target_include_directories(fiber PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/util)
