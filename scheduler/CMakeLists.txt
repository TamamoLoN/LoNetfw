cmake_minimum_required(VERSION 3.1)

link_directories(${CMAKE_SOURCE_DIR}/lib
                 )

include_directories(${CMAKE_SOURCE_DIR}
                    ${CMAKE_SOURCE_DIR}/src
                    ${YAML_INCLUDE_PATH}
                    )

file(GLOB SCHEDULER_SRCS ${CMAKE_SOURCE_DIR}/scheduler/*.cpp)

add_library(scheduler SHARED
            ${SCHEDULER_SRCS}
            )

target_link_libraries(scheduler PRIVATE util thread fiber log config)
target_include_directories(scheduler PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/util)
