cmake_minimum_required(VERSION 3.1)

link_directories(${CMAKE_SOURCE_DIR}/lib
                 )

include_directories(${CMAKE_SOURCE_DIR}
                    ${CMAKE_SOURCE_DIR}/src
                    ${YAML_INCLUDE_PATH}
                    )

file(GLOB HOOK_SRCS ${CMAKE_SOURCE_DIR}/hook/*.cpp)

add_library(hook ${LIB_TYPE}
            ${HOOK_SRCS}
            )

target_link_libraries(hook PRIVATE util thread scheduler dl)
target_include_directories(hook PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/util)

if(USE_ABSOLUTE_PATH)
else()
    force_redefine_file_macro_for_sources(hook)
endif()