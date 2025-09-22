cmake_minimum_required(VERSION 3.1)

link_directories(${CMAKE_SOURCE_DIR}/lib
                 )

include_directories(${CMAKE_SOURCE_DIR}
                    ${CMAKE_SOURCE_DIR}/src
                    ${YAML_INCLUDE_PATH}
                    )

file(GLOB SERVER_SRCS ${CMAKE_SOURCE_DIR}/server/*.cpp)

add_library(server SHARED
            ${SERVER_SRCS}
            )

target_link_libraries(server PRIVATE util scheduler net)
target_include_directories(server PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/util)

if(USE_ABSOLUTE_PATH)
else()
    force_redefine_file_macro_for_sources(server)
endif()