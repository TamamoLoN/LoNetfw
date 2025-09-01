cmake_minimum_required(VERSION 3.1)

link_directories(${CMAKE_SOURCE_DIR}/lib
                 )

include_directories(${CMAKE_SOURCE_DIR}
                    ${CMAKE_SOURCE_DIR}/src
                    ${YAML_INCLUDE_PATH}
                    )

file(GLOB CONFIG_SRCS ${CMAKE_SOURCE_DIR}/config/*.cpp)

add_library(config SHARED
            ${CONFIG_SRCS}
            )

target_link_libraries(config PRIVATE util thread log)
target_include_directories(config PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/util)

if(USE_ABSOLUTE_PATH)
else()
    force_redefine_file_macro_for_sources(config)
endif()