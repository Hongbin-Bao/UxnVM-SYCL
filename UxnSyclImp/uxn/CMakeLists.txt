cmake_minimum_required(VERSION 3.25)

project(uxn)


add_definitions(-D_DARWIN_C_SOURCE)


set(CMAKE_CXX_STANDARD 17)
include(FetchContent)
find_package(SDL2 REQUIRED)
# SYCL
find_package(OpenSYCL REQUIRED)
####
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

if(NOT OPENSYCL_DEBUG_LEVEL)
    if(CMAKE_BUILD_TYPE MATCHES "Debug")
        set(OPENSYCL_DEBUG_LEVEL 3 CACHE STRING
                "Choose the debug level, options are: 0 (no debug), 1 (print errors), 2 (also print warnings), 3 (also print general information)"
                FORCE)
    else()
        set(OPENSYCL_DEBUG_LEVEL 2 CACHE STRING
                "Choose the debug level, options are: 0 (no debug), 1 (print errors), 2 (also print warnings), 3 (also print general information)"
                FORCE)
    endif()
endif()

if(WIN32)
    add_definitions(-D_USE_MATH_DEFINES)
endif()
include_directories(/opt/homebrew/Cellar/libomp/16.0.6/include)

# Fetch and configure Google Test
FetchContent_Declare(
        gtest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.13.0
)

FetchContent_MakeAvailable(gtest)


####
set(UXN_SOURCE_DIR ${PROJECT_SOURCE_DIR}/src)

set(UXN_TEST_SOURCE_DIR ${PROJECT_SOURCE_DIR}/Test)

set(UXN_SOURCE_CORE
        ${UXN_SOURCE_DIR}/uxn.cpp
        ${UXN_SOURCE_DIR}/uxn.h
        )
set(UXN_INFO
        ${UXN_SOURCE_DIR}/uxncli.cpp
        )
source_group("core" FILES ${UXN_SOURCE_CORE})

set(UXN_SOURCE_DEVICE_SYSTEM
        ${UXN_SOURCE_DIR}/devices/system.cpp
        ${UXN_SOURCE_DIR}/devices/system.h
        )
source_group("devices\\system" FILES ${UXN_SOURCE_DEVICE_SYSTEM})

set(UXN_SOURCE_DEVICE_SCREEN
        ${UXN_SOURCE_DIR}/devices/screen.cpp
        ${UXN_SOURCE_DIR}/devices/screen.h
        )
source_group("devices\\screen" FILES ${UXN_SOURCE_DEVICE_SCREEN})

set(UXN_SOURCE_DEVICE_AUDIO
        ${UXN_SOURCE_DIR}/devices/audio.cpp
        ${UXN_SOURCE_DIR}/devices/audio.h
        )
source_group("devices\\audio" FILES ${UXN_SOURCE_DEVICE_AUDIO})

set(UXN_SOURCE_DEVICE_FILE
        ${UXN_SOURCE_DIR}/devices/file.cpp
        ${UXN_SOURCE_DIR}/devices/file.h
        )
source_group("devices\\file" FILES ${UXN_SOURCE_DEVICE_FILE})

set(UXN_SOURCE_DEVICE_CONTROLLER
        ${UXN_SOURCE_DIR}/devices/controller.cpp
        ${UXN_SOURCE_DIR}/devices/controller.h
        )
source_group("devices\\controller" FILES ${UXN_SOURCE_DEVICE_CONTROLLER})

set(UXN_SOURCE_DEVICE_MOUSE
        ${UXN_SOURCE_DIR}/devices/mouse.cpp
        ${UXN_SOURCE_DIR}/devices/mouse.h
        )
source_group("devices\\mouse" FILES ${UXN_SOURCE_DEVICE_MOUSE})

set(UXN_SOURCE_DEVICE_DATETIME
        ${UXN_SOURCE_DIR}/devices/datetime.cpp
        ${UXN_SOURCE_DIR}/devices/datetime.h
        )
source_group("devices\\datetime" FILES ${UXN_SOURCE_DEVICE_DATETIME})

add_executable(uxnemu
        ${UXN_SOURCE_DIR}/uxnemu.cpp
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
        ${UXN_SOURCE_DIR}/uxncli.cpp
        ${UXN_SOURCE_CORE}
        ${UXN_SOURCE_DEVICE_SYSTEM}
        ${UXN_SOURCE_DEVICE_FILE}
        ${UXN_SOURCE_DEVICE_DATETIME}
        )


add_executable(uxnasm
        ${UXN_SOURCE_DIR}/uxnasm.cpp
        )

add_executable(TestExample
        ${UXN_TEST_SOURCE_DIR}/test_example.cpp
        ${UXN_INFO}
        ${UXN_SOURCE_CORE}
        ${UXN_SOURCE_DEVICE_SYSTEM}
        ${UXN_SOURCE_DEVICE_FILE}
        ${UXN_SOURCE_DEVICE_DATETIME}


        )

target_link_libraries(TestExample PRIVATE gtest gtest_main pthread)

add_sycl_to_target(TARGET uxncli SOURCES ${UXN_SOURCE_DIR}/uxncli.cpp
        ${UXN_SOURCE_CORE}
        ${UXN_SOURCE_DEVICE_SYSTEM}
        ${UXN_SOURCE_DEVICE_FILE}
        ${UXN_SOURCE_DEVICE_DATETIME}
        )

add_sycl_to_target(TARGET uxnemu SOURCES ${UXN_SOURCE_DIR}/uxnemu.cpp
        ${UXN_SOURCE_CORE}
        ${UXN_SOURCE_DEVICE_SYSTEM}
        ${UXN_SOURCE_DEVICE_SCREEN}
        ${UXN_SOURCE_DEVICE_AUDIO}
        ${UXN_SOURCE_DEVICE_FILE}
        ${UXN_SOURCE_DEVICE_CONTROLLER}
        ${UXN_SOURCE_DEVICE_MOUSE}
        ${UXN_SOURCE_DEVICE_DATETIME}
        )

add_sycl_to_target(TARGET TestExample SOURCES ${UXN_TEST_SOURCE_DIR}/test_example.cpp
        ${UXN_SOURCE_DIR}/uxncli.cpp
        ${UXN_SOURCE_DIR}/uxnemu.cpp
        ${UXN_SOURCE_DIR}/uxn.cpp
        ${UXN_SOURCE_CORE}/uxnasm.cpp

        ${UXN_SOURCE_CORE}
        ${UXN_SOURCE_DEVICE_SYSTEM}
        ${UXN_SOURCE_DEVICE_SCREEN}
        ${UXN_SOURCE_DEVICE_AUDIO}
        ${UXN_SOURCE_DEVICE_FILE}
        ${UXN_SOURCE_DEVICE_CONTROLLER}
        ${UXN_SOURCE_DEVICE_MOUSE}
        ${UXN_SOURCE_DEVICE_DATETIME})



enable_testing()
add_test(NAME TestExample COMMAND TestExample)