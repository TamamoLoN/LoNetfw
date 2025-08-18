cmake_minimum_required(VERSION 3.1)
set(PROJECT_NAME LoNetfw)
set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/install)
project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

set(CMAKE_BUILD_TYPE Debug)

add_definitions(-g -LINUX_X86)

# set(CMAKE_CXX_COMPILER "aarch64-linux-gnu-g++-8")
#set(CMAKE_CXX_COMPILER "/usr/bin/g++")

set(CMAKE_CXX_FLAGS "-std=c++11 -Wno-builtin-macro-redefined -Wno-terminate -Wno-deprecated-declarations")
set(YAML_INCLUDE_PATH ${CMAKE_SOURCE_DIR}/third_party/yaml-cpp/include)

function(force_redefine_file_macro_for_sources targetname)
    get_target_property (source_files "${targetname}" SOURCES)
    foreach (sourcefile ${source_files})
        #Get source file's current list of compile definitions.
        get_property(defs SOURCE "${sourcefile}"
            PROPERTY COMPILE_DEFINITIONS
            )
        # Get the relative path of the source file in project directory
        get_filename_component(filepath "${sourcefile}" ABSOLUTE)
        string(REPLACE ${PROJECT_SOURCE_DIR}/ "" relpath ${filepath} )
        list(APPEND defs "__FILE__=\"${relpath}\"")
        # Set the updated compile definitions on the source file.
        set_property (
            SOURCE "${sourcefile}"
            PROPERTY COMPILE_DEFINITIONS ${defs}
        )
    endforeach()
endfunction()

function(add_build_target targetname)
    add_executable(${targetname}
                ${CMAKE_SOURCE_DIR}/src/${targetname}.cpp
                )
    target_link_libraries(${targetname} PRIVATE 
                            util 
                            fiber
                            thread
                            log 
                            config 
                            scheduler
                            hook
                            yaml-cpp
                            dl
                            ) 
    # force_redefine_file_macro_for_sources(${targetname})
endfunction()

add_subdirectory(third_party/yaml-cpp)
add_subdirectory(util)
add_subdirectory(log)
add_subdirectory(config)
add_subdirectory(fiber)
add_subdirectory(thread)
add_subdirectory(scheduler)
add_subdirectory(hook)

link_directories(${CMAKE_SOURCE_DIR}/lib
                 )

include_directories(${CMAKE_SOURCE_DIR}
                    ${CMAKE_SOURCE_DIR}/src
                    ${CMAKE_SOURCE_DIR}/third_party
		    ${YAML_INCLUDE_PATH}
                    )

# find_library(YAML_CPP_LIB
#             NAMES yaml-cpp
#             PATHS ${CMAKE_SOURCE_DIR}/third_party/yaml-cpp
#             NO_DEFAULT_PATH 
#             )

# add_executable(${PROJECT_NAME}
#                ${CMAKE_SOURCE_DIR}/src/main.cpp
#                )

# target_link_libraries(${PROJECT_NAME} PRIVATE util thread log config) 
# force_redefine_file_macro_for_sources(${PROJECT_NAME})

add_build_target(test_log)

add_build_target(test_config)

add_build_target(test_thread)

add_build_target(test_util)

add_build_target(test_fiber)

add_build_target(test_scheduler)

add_build_target(test_ioscheduler)

add_build_target(test_hook)
