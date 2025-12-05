cmake_minimum_required(VERSION 3.1)

link_directories(${CMAKE_SOURCE_DIR}/lib
                 )

include_directories(${CMAKE_SOURCE_DIR}
                    ${CMAKE_SOURCE_DIR}/src
                    ${YAML_INCLUDE_PATH}
                    )

file(GLOB THREAD_SRCS ${CMAKE_SOURCE_DIR}/thread/*.cpp)

add_library(thread ${LIB_TYPE}
            ${THREAD_SRCS}
            )

if(WIN32)
    target_link_libraries(thread PRIVATE util pthreadVC2)
else()
    target_link_libraries(thread PRIVATE util pthread)
endif()

target_include_directories(thread PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/util)

if(USE_ABSOLUTE_PATH)
else()
    force_redefine_file_macro_for_sources(thread)
endif()