cmake_minimum_required(VERSION 3.1)

link_directories(${CMAKE_SOURCE_DIR}/lib
                 )

include_directories(${CMAKE_SOURCE_DIR}
                    ${CMAKE_SOURCE_DIR}/src
                    ${YAML_INCLUDE_PATH}
                    )

file(GLOB LOG_SRCS ${CMAKE_SOURCE_DIR}/log/*.cpp)

add_library(log ${LIB_TYPE}
            ${LOG_SRCS}
            )

target_link_libraries(log PRIVATE util fiber thread)
target_include_directories(log PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/util)

if(USE_ABSOLUTE_PATH)
else()
    force_redefine_file_macro_for_sources(log)
endif()
