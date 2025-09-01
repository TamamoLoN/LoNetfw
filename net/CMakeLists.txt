cmake_minimum_required(VERSION 3.1)

link_directories(${CMAKE_SOURCE_DIR}/lib
                 )

include_directories(${CMAKE_SOURCE_DIR}
                    ${CMAKE_SOURCE_DIR}/src
                    ${YAML_INCLUDE_PATH}
                    )

file(GLOB NET_SRCS ${CMAKE_SOURCE_DIR}/net/*.cpp)

add_library(net SHARED
            ${NET_SRCS}
            )

target_link_libraries(net PRIVATE util log hook dl)
target_include_directories(net PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/util)

if(USE_ABSOLUTE_PATH)
else()
    force_redefine_file_macro_for_sources(net)
endif()