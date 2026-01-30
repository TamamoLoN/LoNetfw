cmake_minimum_required(VERSION 3.1)

link_directories(${CMAKE_SOURCE_DIR}/lib
                 )

include_directories(${CMAKE_SOURCE_DIR}
                    ${CMAKE_SOURCE_DIR}/src
                    ${YAML_INCLUDE_PATH}
                    )

file(GLOB SYSTEM_SRCS ${CMAKE_SOURCE_DIR}/system/*.cpp)

add_library(system ${LIB_TYPE}
            ${SYSTEM_SRCS}
            )

target_link_libraries(system PRIVATE util log server)
target_include_directories(system PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/util)

if(USE_ABSOLUTE_PATH)
else()
    force_redefine_file_macro_for_sources(system)
endif()