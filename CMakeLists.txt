cmake_minimum_required(VERSION 3.1)
set(PROJECT_NAME lonetfw)
set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/install/${PROJECT_NAME})
project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
# set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
# set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)

set(CMAKE_BUILD_TYPE Debug)

add_definitions(-g -LINUX_X86)

# set(CMAKE_CXX_COMPILER "aarch64-linux-gnu-g++-8")
#set(CMAKE_CXX_COMPILER "/usr/bin/g++")

### options ###
option(USE_HIGH_PRECISION_TIME "Enable high precision time" ON)
option(USE_ABSOLUTE_PATH "Enable show absolute path in log" OFF)
#当使用MODULARIZATION模块化后，如果想要函数被hook住，请在使用处添加hook/hook.h头文件
option(MODULARIZATION 
        "Enable modularization[module hook must be used in src or main, if not then some modules may can not be hooked sucessfully]" 
        ON)
option(BUILT_TESTS "Enable to build tests" ON)
###############

if(USE_HIGH_PRECISION_TIME)
    add_definitions(-DUSE_HIGH_PRECISION_TIME)
endif()

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
                ${CMAKE_SOURCE_DIR}/tests/${targetname}.cpp
                )
    if(MODULARIZATION)
        target_link_libraries(${targetname} PRIVATE 
                                util 
                                fiber
                                thread
                                log 
                                config 
                                scheduler
                                hook
                                net
                                yaml-cpp
                                dl
                                ) 
    else()
        target_link_libraries(${targetname} PRIVATE 
                                    ${PROJECT_NAME}
                                    yaml-cpp
                                    dl
                                    ) 
    endif()
    if(USE_ABSOLUTE_PATH)
    else()
        force_redefine_file_macro_for_sources(${targetname})
    endif()
    
endfunction()

if(MODULARIZATION)
    add_subdirectory(third_party/yaml-cpp)
    add_subdirectory(util)
    add_subdirectory(log)
    add_subdirectory(config)
    add_subdirectory(fiber)
    add_subdirectory(thread)
    add_subdirectory(scheduler)
    add_subdirectory(hook)
    add_subdirectory(net)
else()
    add_subdirectory(third_party/yaml-cpp)
    file(GLOB MODULE_SRCS 
        util/*.cpp
        log/*.cpp
        config/*.cpp
        fiber/*.cpp
        thread/*.cpp
        scheduler/*.cpp
        hook/*.cpp
        net/*.cpp
    )
    add_library(${PROJECT_NAME} SHARED ${MODULE_SRCS})
    target_link_libraries(${PROJECT_NAME} PRIVATE pthread dl yaml-cpp) 
    if(USE_ABSOLUTE_PATH)
    else()
        force_redefine_file_macro_for_sources(${PROJECT_NAME})
    endif()
endif()


link_directories(${CMAKE_SOURCE_DIR}/lib
                 )

include_directories(${CMAKE_SOURCE_DIR}
                    ${CMAKE_SOURCE_DIR}/src
                    ${CMAKE_SOURCE_DIR}/third_party
		    ${YAML_INCLUDE_PATH}
                    )

if(BUILT_TESTS)
    add_build_target(test_log)
    add_build_target(test_config)
    add_build_target(test_thread)
    add_build_target(test_util)
    add_build_target(test_fiber)
    add_build_target(test_scheduler)
    add_build_target(test_ioscheduler)
    add_build_target(test_hook)
    add_build_target(test_net)
endif()

if(MODULARIZATION)
    install(TARGETS 
            util
            fiber
            thread
            log
            config
            scheduler
            hook
            net
            ARCHIVE DESTINATION lib 
            LIBRARY DESTINATION lib 
            RUNTIME DESTINATION bin)
else()
    install(TARGETS ${PROJECT_NAME}
            ARCHIVE DESTINATION lib 
            LIBRARY DESTINATION lib 
            RUNTIME DESTINATION bin)
endif()

install(DIRECTORY 
        util
        fiber
        thread
        log
        config
        scheduler
        hook
        net
        DESTINATION include
        FILES_MATCHING
        PATTERN "*.h"
        PATTERN "*.hpp"
)

install(FILES lonetfw/lonetfw.h DESTINATION include)
