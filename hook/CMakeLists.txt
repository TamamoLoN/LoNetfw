cmake_minimum_required(VERSION 3.1)

link_directories(${CMAKE_SOURCE_DIR}/lib
                ${CMAKE_SOURCE_DIR}/third_party/yaml-cpp
                 )

include_directories(${CMAKE_SOURCE_DIR}
                    ${CMAKE_SOURCE_DIR}/src
                    ${CMAKE_SOURCE_DIR}/third_party
                    third_party
                    )

file(GLOB HOOK_SRCS ${CMAKE_SOURCE_DIR}/hook/*.cpp)

add_library(hook SHARED
            ${HOOK_SRCS}
            )

target_link_libraries(hook PRIVATE util scheduler)
target_include_directories(hook PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/util)