cmake_minimum_required(VERSION 3.24)

project(WhatSon VERSION 0.1.0 LANGUAGES CXX)

include(CTest)

set(WHATSON_IOS_SIMULATOR_ARCH "arm64" CACHE STRING
        "Preferred iOS simulator architecture.")

if (CMAKE_SYSTEM_NAME STREQUAL "iOS")
    if (NOT DEFINED CMAKE_OSX_ARCHITECTURES OR CMAKE_OSX_ARCHITECTURES STREQUAL "")
        if (CMAKE_OSX_SYSROOT MATCHES "iphonesimulator")
            set(CMAKE_OSX_ARCHITECTURES "${WHATSON_IOS_SIMULATOR_ARCH}" CACHE STRING "iOS simulator architectures" FORCE)
        elseif (CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "arm64")
            set(CMAKE_OSX_ARCHITECTURES "arm64" CACHE STRING "iOS device architectures" FORCE)
        endif ()
    endif ()
endif ()

if (APPLE AND NOT CMAKE_SYSTEM_NAME STREQUAL "iOS")
    if (NOT DEFINED LVRS_BOOTSTRAP_IOS_ARCHITECTURES OR LVRS_BOOTSTRAP_IOS_ARCHITECTURES STREQUAL "")
        set(LVRS_BOOTSTRAP_IOS_ARCHITECTURES "${WHATSON_IOS_SIMULATOR_ARCH}" CACHE STRING
                "LVRS bootstrap iOS simulator architectures." FORCE)
    endif ()
endif ()


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Add local prefixes for dependency discovery.
# LVRS root package now dispatches platform-specific configs internally.
if (NOT LVRS_PREFIX AND EXISTS "$ENV{HOME}/.local/LVRS")
    set(LVRS_PREFIX "$ENV{HOME}/.local/LVRS" CACHE PATH "Path to LVRS root prefix")
endif ()
if (LVRS_PREFIX)
    list(APPEND CMAKE_PREFIX_PATH "${LVRS_PREFIX}")
endif ()

if (APPLE)
    # Common Qt installation paths on macOS
    if (NOT QT_ROOT_PATH)
        file(GLOB QT_INSTALL_DIRS "$ENV{HOME}/Qt/6.*")
        if (QT_INSTALL_DIRS)
            list(SORT QT_INSTALL_DIRS ORDER DESCENDING)
            list(GET QT_INSTALL_DIRS 0 LATEST_QT)
            set(QT_ROOT_PATH "${LATEST_QT}/macos" CACHE PATH "Path to Qt installation")
        endif ()
    endif ()
endif ()

if (QT_ROOT_PATH)
    list(APPEND CMAKE_PREFIX_PATH "${QT_ROOT_PATH}")
endif ()

option(WHATSON_BUILD_APP "Build the Qt Quick desktop app" ON)
option(WHATSON_BUILD_DAEMON "Build the background daemon" ON)
option(WHATSON_BUILD_CLI "Build the Rust CLI launcher" ON)
option(WHATSON_ENABLE_DEV_TOOLING "Enable developer lint/format convenience targets" ON)
get_filename_component(_whatson_binary_dir_name "${CMAKE_BINARY_DIR}" NAME)
set(_WHATSON_IS_TRIAL_BUILD_DEFAULT OFF)
if (_whatson_binary_dir_name STREQUAL "build-trial")
    set(_WHATSON_IS_TRIAL_BUILD_DEFAULT ON)
endif ()
option(WHATSON_IS_TRIAL_BUILD "Mark this build tree as the dedicated trial packaging tree." ${_WHATSON_IS_TRIAL_BUILD_DEFAULT})
# Keep nested trial packaging opt-in so ordinary host edit/build loops stay incremental.
set(_WHATSON_ENABLE_TRIAL_BUILD_MIRROR_DEFAULT OFF)
option(WHATSON_ENABLE_TRIAL_BUILD_MIRROR
        "Configure and build the dedicated build-trial packaging tree from this host build."
        ${_WHATSON_ENABLE_TRIAL_BUILD_MIRROR_DEFAULT})
set(WHATSON_TRIAL_BUILD_DIR "${CMAKE_SOURCE_DIR}/build-trial" CACHE PATH
        "Output directory for the dedicated desktop trial packaging tree.")
set(_WHATSON_TRIAL_BUILD_OSX_ARCHITECTURES_DEFAULT "")
if (APPLE AND NOT CMAKE_SYSTEM_NAME STREQUAL "iOS")
    set(_whatson_trial_requested_osx_architectures "")
    if (DEFINED CMAKE_OSX_ARCHITECTURES AND NOT CMAKE_OSX_ARCHITECTURES STREQUAL "")
        set(_whatson_trial_requested_osx_architectures "${CMAKE_OSX_ARCHITECTURES}")
    else ()
        execute_process(
                COMMAND /usr/bin/uname -m
                RESULT_VARIABLE _whatson_trial_uname_result
                OUTPUT_VARIABLE _whatson_trial_uname_architecture
                OUTPUT_STRIP_TRAILING_WHITESPACE
                ERROR_QUIET
        )
        if (_whatson_trial_uname_result EQUAL 0 AND NOT _whatson_trial_uname_architecture STREQUAL "")
            set(_whatson_trial_requested_osx_architectures "${_whatson_trial_uname_architecture}")
        endif ()
    endif ()

    set(_whatson_trial_lvrs_library "")
    if (LVRS_PREFIX AND EXISTS "${LVRS_PREFIX}/platforms/macos/lib/libLVRS.dylib")
        set(_whatson_trial_lvrs_library "${LVRS_PREFIX}/platforms/macos/lib/libLVRS.dylib")
    endif ()

    set(_whatson_trial_lvrs_architectures "")
    if (_whatson_trial_lvrs_library)
        find_program(_WHATSON_LIPO_EXECUTABLE NAMES lipo)
        if (_WHATSON_LIPO_EXECUTABLE)
            execute_process(
                    COMMAND "${_WHATSON_LIPO_EXECUTABLE}" -archs "${_whatson_trial_lvrs_library}"
                    RESULT_VARIABLE _whatson_trial_lipo_result
                    OUTPUT_VARIABLE _whatson_trial_lvrs_architectures_raw
                    OUTPUT_STRIP_TRAILING_WHITESPACE
                    ERROR_QUIET
            )
            if (_whatson_trial_lipo_result EQUAL 0 AND NOT _whatson_trial_lvrs_architectures_raw STREQUAL "")
                string(REGEX REPLACE "[ \t]+" ";" _whatson_trial_lvrs_architectures
                        "${_whatson_trial_lvrs_architectures_raw}")
            endif ()
        endif ()
    endif ()

    if (_whatson_trial_requested_osx_architectures)
        string(REGEX REPLACE "[;, ]+" ";" _whatson_trial_requested_osx_architecture_list
                "${_whatson_trial_requested_osx_architectures}")
    else ()
        set(_whatson_trial_requested_osx_architecture_list "")
    endif ()

    if (_whatson_trial_lvrs_architectures)
        set(_whatson_trial_effective_osx_architectures "")
        foreach (_whatson_trial_requested_architecture IN LISTS _whatson_trial_requested_osx_architecture_list)
            list(FIND _whatson_trial_lvrs_architectures "${_whatson_trial_requested_architecture}"
                    _whatson_trial_lvrs_architecture_index)
            if (NOT _whatson_trial_lvrs_architecture_index EQUAL -1)
                list(APPEND _whatson_trial_effective_osx_architectures
                        "${_whatson_trial_requested_architecture}")
            endif ()
        endforeach ()

        if (_whatson_trial_effective_osx_architectures)
            list(REMOVE_DUPLICATES _whatson_trial_effective_osx_architectures)
            list(JOIN _whatson_trial_effective_osx_architectures ";"
                    _WHATSON_TRIAL_BUILD_OSX_ARCHITECTURES_DEFAULT)
        else ()
            list(GET _whatson_trial_lvrs_architectures 0 _WHATSON_TRIAL_BUILD_OSX_ARCHITECTURES_DEFAULT)
        endif ()
    else ()
        set(_WHATSON_TRIAL_BUILD_OSX_ARCHITECTURES_DEFAULT "${_whatson_trial_requested_osx_architectures}")
    endif ()
endif ()
set(WHATSON_TRIAL_BUILD_OSX_ARCHITECTURES "${_WHATSON_TRIAL_BUILD_OSX_ARCHITECTURES_DEFAULT}" CACHE STRING
        "macOS architectures used when configuring the dedicated build-trial tree.")
if (WHATSON_IS_TRIAL_BUILD)
    set(WHATSON_ENABLE_TRIAL_BUILD_MIRROR OFF CACHE BOOL
            "Configure and build the dedicated build-trial packaging tree from this host build." FORCE)
endif ()

set(WHATSON_PLATFORM_DIR "${CMAKE_SOURCE_DIR}/platform")
set(WHATSON_RESOURCES_DIR "${CMAKE_SOURCE_DIR}/resources")
set(WHATSON_APP_ICON_DIR "${WHATSON_RESOURCES_DIR}/icons/app")
set(WHATSON_APP_ICON_DESKTOP_DIR "${WHATSON_APP_ICON_DIR}/desktop")
set(WHATSON_APPLE_DIR "${WHATSON_PLATFORM_DIR}/Apple")
set(WHATSON_ANDROID_DIR "${WHATSON_PLATFORM_DIR}/Android")
set(WHATSON_LINUX_DIR "${WHATSON_PLATFORM_DIR}/Linux")
set(WHATSON_IOS_INFO_PLIST "${WHATSON_APPLE_DIR}/iOS/Info.plist")
set(WHATSON_DEFAULT_INFO_PLIST "${WHATSON_APPLE_DIR}/Info.plist")
set(WHATSON_ENTITLEMENTS "${WHATSON_APPLE_DIR}/WhatSon.entitlements")
set(WHATSON_APP_ICON_PNG "${WHATSON_APP_ICON_DESKTOP_DIR}/AppIcon.png")
set(WHATSON_APPLE_BUNDLE_ID "com.iisacc.app.whatson" CACHE STRING "Apple bundle id for WhatSon app")
set(WHATSON_ANDROID_PACKAGE_ID "com.iisacc.app.whatson" CACHE STRING "Android package id for WhatSon app")
set(WHATSON_IOS_XCODEPROJ_DIR "${CMAKE_BINARY_DIR}/ios-xcode-artifact" CACHE PATH
        "Output directory for generated iOS Xcode project.")

set(_WHATSON_ENABLE_IOS_XCODEPROJ_DEFAULT OFF)
if (APPLE AND NOT CMAKE_SYSTEM_NAME STREQUAL "iOS")
    set(_WHATSON_ENABLE_IOS_XCODEPROJ_DEFAULT ON)
endif ()
set(WHATSON_ENABLE_IOS_XCODEPROJ_ON_BUILD "${_WHATSON_ENABLE_IOS_XCODEPROJ_DEFAULT}" CACHE BOOL
        "Enable the explicit iOS Xcode project export target on Apple hosts.")

find_package(Qt6 6.5 REQUIRED COMPONENTS Core)
qt_standard_project_setup(REQUIRES 6.5)

message(STATUS "WhatSon configuration")
message(STATUS "  WHATSON_BUILD_APP: ${WHATSON_BUILD_APP}")
message(STATUS "  WHATSON_BUILD_DAEMON: ${WHATSON_BUILD_DAEMON}")
message(STATUS "  WHATSON_BUILD_CLI: ${WHATSON_BUILD_CLI}")
message(STATUS "  WHATSON_ENABLE_DEV_TOOLING: ${WHATSON_ENABLE_DEV_TOOLING}")
message(STATUS "  BUILD_TESTING: ${BUILD_TESTING}")
message(STATUS "  WHATSON_IS_TRIAL_BUILD: ${WHATSON_IS_TRIAL_BUILD}")
message(STATUS "  WHATSON_ENABLE_TRIAL_BUILD_MIRROR: ${WHATSON_ENABLE_TRIAL_BUILD_MIRROR}")
message(STATUS "  WHATSON_TRIAL_BUILD_DIR: ${WHATSON_TRIAL_BUILD_DIR}")
message(STATUS "  WHATSON_TRIAL_BUILD_OSX_ARCHITECTURES: ${WHATSON_TRIAL_BUILD_OSX_ARCHITECTURES}")
message(STATUS "  CMAKE_PREFIX_PATH: ${CMAKE_PREFIX_PATH}")
message(STATUS "  QT_ROOT_PATH: ${QT_ROOT_PATH}")
message(STATUS "  LVRS_PREFIX: ${LVRS_PREFIX}")

if (WHATSON_BUILD_APP)
    add_subdirectory(src/app)
endif ()

if (WHATSON_BUILD_DAEMON)
    add_subdirectory(src/daemon)
endif ()

if (WHATSON_BUILD_CLI)
    add_subdirectory(src/cli)
endif ()

if (BUILD_TESTING)
    add_subdirectory(test)
endif ()

set(WHATSON_BUILD_TARGETS)
if (TARGET WhatSon)
    list(APPEND WHATSON_BUILD_TARGETS WhatSon)
endif ()
if (TARGET WhatSon_daemon)
    list(APPEND WHATSON_BUILD_TARGETS WhatSon_daemon)
endif ()
if (TARGET whatson_cli)
    list(APPEND WHATSON_BUILD_TARGETS whatson_cli)
endif ()

if (WHATSON_BUILD_TARGETS)
    add_custom_target(whatson_build_all DEPENDS ${WHATSON_BUILD_TARGETS})
endif ()

add_subdirectory(cmake/root)
