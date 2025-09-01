cmake_minimum_required(VERSION 3.1)

link_directories(${CMAKE_SOURCE_DIR}/lib
                 )

include_directories(${CMAKE_SOURCE_DIR}
                    ${CMAKE_SOURCE_DIR}/src
                    ${YAML_INCLUDE_PATH}
                    )

file(GLOB UTIL_SRCS ${CMAKE_SOURCE_DIR}/util/*.cpp)

add_library(util SHARED
            ${UTIL_SRCS}
            )

target_include_directories(util PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(util PRIVATE yaml-cpp pthread dl)

if(USE_ABSOLUTE_PATH)
else()
    force_redefine_file_macro_for_sources(util)
endif()