cmake_minimum_required(VERSION 3.1)

link_directories(${CMAKE_SOURCE_DIR}/lib
                 )

include_directories(${CMAKE_SOURCE_DIR}
                    ${CMAKE_SOURCE_DIR}/src
                    ${YAML_INCLUDE_PATH}
                    )

file(GLOB SOCKET_SRCS ${CMAKE_SOURCE_DIR}/socket/*.cpp)

add_library(socket SHARED
            ${SOCKET_SRCS}
            )

target_link_libraries(socket PRIVATE util log)
target_include_directories(socket PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/util)
