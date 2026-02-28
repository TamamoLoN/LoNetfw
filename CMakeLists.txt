cmake_minimum_required(VERSION 3.1)
set(PROJECT_NAME lonetfw)
set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/install/${PROJECT_NAME})
project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
# set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
# set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)

set(CMAKE_BUILD_TYPE Debug)
set(LIB_TYPE SHARED)

if(WIN32)
    message(STATUS "Building on Windows (MSVC)")
    add_definitions(/DWIN32 /D_WINDOWS)
    add_definitions(/D_CRT_SECURE_NO_WARNINGS)
    add_compile_options(/Zi)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/minhook)
    include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/win32/pthreads-w32/include
        ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/win32/wepoll/include
        ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/minhook/include
    )
    link_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/win32/pthreads-w32/lib/x64
        ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/win32/wepoll/lib
        ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/win32/dl/lib
    )
    set(LIB_TYPE STATIC)
else()
    message(STATUS "Building on Linux")
    add_definitions(-g -LINUX_X86)
    set(CMAKE_CXX_FLAGS "-std=c++11 -O3 -Wno-builtin-macro-redefined -Wno-terminate -Wno-deprecated-declarations")
endif()

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

set(YAML_INCLUDE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/yaml-cpp/include)
set(OPENSSL_INCLUDE_PATH ${CMAKE_CURRENT_BINARY_DIR}/thirdparty/openssl/include
)

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
                ${CMAKE_CURRENT_SOURCE_DIR}/tests/${targetname}.cpp
                )
    if(MODULARIZATION)
    if(WIN32)
        target_link_libraries(${targetname} PRIVATE 
                                util 
                                fiber
                                thread
                                log 
                                config 
                                scheduler
                                hook
                                net
                                server
                                system
                                yaml-cpp
                                dl
                                ws2_32
                                minhook
                                wepoll
                                iphlpapi
                                ) 
    else()
        target_link_libraries(${targetname} PRIVATE 
                                util 
                                fiber
                                thread
                                log 
                                config 
                                scheduler
                                hook
                                net
                                server
                                system
                                yaml-cpp
                                dl
                                ) 
    endif()
    else()
        target_link_libraries(${targetname} PRIVATE 
                                    ${PROJECT_NAME}
                                    ) 
    endif()
    if(USE_ABSOLUTE_PATH)
    else()
        force_redefine_file_macro_for_sources(${targetname})
    endif()
    
endfunction()

set(YAML_CPP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(YAML_CPP_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
set(YAML_CPP_BUILD_CONTRIB OFF CACHE BOOL "" FORCE)
# yaml 0.6.0
set(BUILD_SHARED_LIBS ON CACHE BOOL "" FORCE)
# yaml 0.6.3
set(YAML_BUILD_SHARED_LIBS ON CACHE BOOL "" FORCE)

set(WITH_APPS OFF CACHE BOOL "" FORCE)

link_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib
                 )

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_SOURCE_DIR}/src
                    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty
		            ${YAML_INCLUDE_PATH}
                    ${OPENSSL_INCLUDE_PATH}
                    )

add_subdirectory(thirdparty/openssl)

if(MODULARIZATION)
    add_subdirectory(thirdparty/yaml-cpp)
    add_subdirectory(util)
    add_subdirectory(log)
    add_subdirectory(config)
    add_subdirectory(fiber)
    add_subdirectory(thread)
    add_subdirectory(scheduler)
    add_subdirectory(hook)
    add_subdirectory(net)
    add_subdirectory(server)
    add_subdirectory(system)
else()
if(WIN32)
    add_subdirectory(thirdparty/yaml-cpp)
    file(GLOB MODULE_SRCS 
        util/*.cpp
        log/*.cpp
        config/*.cpp
        fiber/*.cpp
        thread/*.cpp
        scheduler/*.cpp
        hook/*.cpp
        net/*.cpp
        server/*.cpp
        system/*.cpp
    )
    add_library(${PROJECT_NAME} ${LIB_TYPE} ${MODULE_SRCS})
    target_link_libraries(${PROJECT_NAME} PUBLIC dl yaml-cpp ws2_32 minhook wepoll iphlpapi pthreadVC2) 
    if(USE_ABSOLUTE_PATH)
    else()
        force_redefine_file_macro_for_sources(${PROJECT_NAME})
    endif()
else()
    add_subdirectory(thirdparty/yaml-cpp)
    file(GLOB MODULE_SRCS 
        util/*.cpp
        log/*.cpp
        config/*.cpp
        fiber/*.cpp
        thread/*.cpp
        scheduler/*.cpp
        hook/*.cpp
        net/*.cpp
        server/*.cpp
        system/*.cpp
    )
    add_library(${PROJECT_NAME} ${LIB_TYPE} ${MODULE_SRCS})
    target_link_libraries(${PROJECT_NAME} PUBLIC pthread dl yaml-cpp ssl) 
    if(USE_ABSOLUTE_PATH)
    else()
        force_redefine_file_macro_for_sources(${PROJECT_NAME})
    endif()
endif()
endif()

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
    add_build_target(test_server)
    add_build_target(test_system)
    add_build_target(test_udp)
    add_build_target(test_application)
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
            server
            system
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
        server
        system
        lonetfw
        DESTINATION include
        FILES_MATCHING
        PATTERN "*.h"
        PATTERN "*.hpp"
)

# install(FILES lonetfw/lonetfw.h DESTINATION include)
