cmake_minimum_required(VERSION 3.1)
set(PROJECT_NAME LoNetfw)
project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

set(CMAKE_BUILD_TYPE Debug)

add_definitions(-g -LINUX_X86)

# set(CMAKE_CXX_COMPILER "aarch64-linux-gnu-g++-8")
set(CMAKE_CXX_COMPILER "g++")

set(CMAKE_CXX_FLAGS "-std=c++11 -Wno-builtin-macro-redefined")

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


add_subdirectory(util)
add_subdirectory(log)
add_subdirectory(config)

link_directories(${CMAKE_SOURCE_DIR}/lib
                 )

include_directories(${CMAKE_SOURCE_DIR}
                    ${CMAKE_SOURCE_DIR}/src
                    # ${CMAKE_SOURCE_DIR}/third_party
                    )

# find_library(YAML_CPP_LIB
#             NAMES yaml-cpp
#             PATHS ${CMAKE_SOURCE_DIR}/third_party/yaml-cpp
#             NO_DEFAULT_PATH 
#             )

add_executable(${PROJECT_NAME}
               ${CMAKE_SOURCE_DIR}/src/main.cpp
               )

target_link_libraries(${PROJECT_NAME} PRIVATE util log config) 
force_redefine_file_macro_for_sources(${PROJECT_NAME})

add_executable(test_config
               ${CMAKE_SOURCE_DIR}/src/test_config.cpp
               )
target_link_libraries(test_config PRIVATE 
                        util 
                        log 
                        config 
                        yaml-cpp
                        # ${YAML_CPP_LIB}
                        pthread
                        ) 
force_redefine_file_macro_for_sources(test_config)