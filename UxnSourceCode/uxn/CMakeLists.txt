cmake_minimum_required(VERSION 3.13)

project(uxn)


add_definitions(-D_DARWIN_C_SOURCE)


set(CMAKE_C_STANDARD 99)

find_package(SDL2 REQUIRED)
## SYCL
#find_package(OpenSYCL REQUIRED)
#####
#if(NOT CMAKE_BUILD_TYPE)
#    set(CMAKE_BUILD_TYPE Release)
#endif()
#
#if(NOT OPENSYCL_DEBUG_LEVEL)
#    if(CMAKE_BUILD_TYPE MATCHES "Debug")
#        set(OPENSYCL_DEBUG_LEVEL 3 CACHE STRING
#                "Choose the debug level, options are: 0 (no debug), 1 (print errors), 2 (also print warnings), 3 (also print general information)"
#                FORCE)
#    else()
#        set(OPENSYCL_DEBUG_LEVEL 2 CACHE STRING
#                "Choose the debug level, options are: 0 (no debug), 1 (print errors), 2 (also print warnings), 3 (also print general information)"
#                FORCE)
#    endif()
#endif()
#
#if(WIN32)
#    add_definitions(-D_USE_MATH_DEFINES)
#endif()
#include_directories(/opt/homebrew/Cellar/libomp/16.0.6/include)
#####
set(UXN_SOURCE_DIR ${PROJECT_SOURCE_DIR}/src)

set(UXN_SOURCE_CORE
        ${UXN_SOURCE_DIR}/uxn.c
        ${UXN_SOURCE_DIR}/uxn.h
        )
source_group("core" FILES ${UXN_SOURCE_CORE})

set(UXN_SOURCE_DEVICE_SYSTEM
        ${UXN_SOURCE_DIR}/devices/system.c
        ${UXN_SOURCE_DIR}/devices/system.h
        )
source_group("devices\\system" FILES ${UXN_SOURCE_DEVICE_SYSTEM})

set(UXN_SOURCE_DEVICE_SCREEN
        ${UXN_SOURCE_DIR}/devices/screen.c
        ${UXN_SOURCE_DIR}/devices/screen.h
        )
source_group("devices\\screen" FILES ${UXN_SOURCE_DEVICE_SCREEN})

set(UXN_SOURCE_DEVICE_AUDIO
        ${UXN_SOURCE_DIR}/devices/audio.c
        ${UXN_SOURCE_DIR}/devices/audio.h
        )
source_group("devices\\audio" FILES ${UXN_SOURCE_DEVICE_AUDIO})

set(UXN_SOURCE_DEVICE_FILE
        ${UXN_SOURCE_DIR}/devices/file.c
        ${UXN_SOURCE_DIR}/devices/file.h
        )
source_group("devices\\file" FILES ${UXN_SOURCE_DEVICE_FILE})

set(UXN_SOURCE_DEVICE_CONTROLLER
        ${UXN_SOURCE_DIR}/devices/controller.c
        ${UXN_SOURCE_DIR}/devices/controller.h
        )
source_group("devices\\controller" FILES ${UXN_SOURCE_DEVICE_CONTROLLER})

set(UXN_SOURCE_DEVICE_MOUSE
        ${UXN_SOURCE_DIR}/devices/mouse.c
        ${UXN_SOURCE_DIR}/devices/mouse.h
        )
source_group("devices\\mouse" FILES ${UXN_SOURCE_DEVICE_MOUSE})

set(UXN_SOURCE_DEVICE_DATETIME
        ${UXN_SOURCE_DIR}/devices/datetime.c
        ${UXN_SOURCE_DIR}/devices/datetime.h
        )
source_group("devices\\datetime" FILES ${UXN_SOURCE_DEVICE_DATETIME})

add_executable(uxnemu
        ${UXN_SOURCE_DIR}/uxnemu.c
        ${UXN_SOURCE_CORE}
        ${UXN_SOURCE_DEVICE_SYSTEM}
        ${UXN_SOURCE_DEVICE_SCREEN}
        ${UXN_SOURCE_DEVICE_AUDIO}
        ${UXN_SOURCE_DEVICE_FILE}
        ${UXN_SOURCE_DEVICE_CONTROLLER}
        ${UXN_SOURCE_DEVICE_MOUSE}
        ${UXN_SOURCE_DEVICE_DATETIME}
        )
target_include_directories(uxnemu PRIVATE ${SDL2_INCLUDE_DIR})
target_link_libraries(uxnemu PRIVATE ${SDL2_LIBRARIES})

add_executable(uxncli
        ${UXN_SOURCE_DIR}/uxncli.c
        ${UXN_SOURCE_CORE}
        ${UXN_SOURCE_DEVICE_SYSTEM}
        ${UXN_SOURCE_DEVICE_FILE}
        ${UXN_SOURCE_DEVICE_DATETIME}
        )


add_executable(uxnasm
        ${UXN_SOURCE_DIR}/uxnasm.c
        )


#add_sycl_to_target(TARGET uxnasm SOURCES uxnasm.cpp)
#add_sycl_to_target(TARGET uxnemu SOURCES uxnemu.cpp)
#add_sycl_to_target(TARGET uxncli SOURCES uxncli.cpp)
