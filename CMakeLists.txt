cmake_minimum_required(VERSION 3.24)

project(WhatSon VERSION 0.1.0 LANGUAGES CXX)

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

function(whatson_runtime_platform out_var)
    if (CMAKE_SYSTEM_NAME STREQUAL "Android")
        set(_whatson_platform "android")
    elseif (CMAKE_SYSTEM_NAME STREQUAL "iOS")
        set(_whatson_platform "ios")
    elseif (APPLE)
        set(_whatson_platform "macos")
    elseif (WIN32)
        set(_whatson_platform "windows")
    elseif (UNIX)
        set(_whatson_platform "linux")
    else ()
        set(_whatson_platform "unknown")
    endif ()
    set(${out_var} "${_whatson_platform}" PARENT_SCOPE)
endfunction()

function(whatson_resolve_lvrs_prefix out_var)
    whatson_runtime_platform(_whatson_runtime_platform)
    set(_whatson_lvrs_prefix "${LVRS_PREFIX}")
    set(_whatson_home "")
    if (DEFINED ENV{HOME} AND NOT "$ENV{HOME}" STREQUAL "")
        set(_whatson_home "$ENV{HOME}")
    elseif (DEFINED ENV{USERPROFILE} AND NOT "$ENV{USERPROFILE}" STREQUAL "")
        set(_whatson_home "$ENV{USERPROFILE}")
    endif ()

    if (_whatson_lvrs_prefix STREQUAL "")
        if (NOT _whatson_home STREQUAL "" AND EXISTS "${_whatson_home}/.local/LVRS")
            set(_whatson_lvrs_prefix "${_whatson_home}/.local/LVRS")
        endif ()
    endif ()

    if (NOT _whatson_lvrs_prefix STREQUAL "" AND EXISTS "${_whatson_lvrs_prefix}/platforms/${_whatson_runtime_platform}")
        set(_whatson_lvrs_prefix "${_whatson_lvrs_prefix}/platforms/${_whatson_runtime_platform}")
    endif ()

    set(${out_var} "${_whatson_lvrs_prefix}" PARENT_SCOPE)
endfunction()

# Add local prefixes for dependency discovery
whatson_resolve_lvrs_prefix(_whatson_lvrs_prefix_resolved)
if (NOT _whatson_lvrs_prefix_resolved STREQUAL "")
    set(LVRS_PREFIX "${_whatson_lvrs_prefix_resolved}" CACHE PATH "Path to LVRS prefix" FORCE)
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

    if (QT_ROOT_PATH)
        list(APPEND CMAKE_PREFIX_PATH "${QT_ROOT_PATH}")
    endif ()
endif ()

option(WHATSON_BUILD_APP "Build the Qt Quick desktop app" ON)
option(WHATSON_BUILD_DAEMON "Build the background daemon" ON)

set(WHATSON_PLATFORM_DIR "${CMAKE_SOURCE_DIR}/platform")
set(WHATSON_APPLE_DIR "${WHATSON_PLATFORM_DIR}/Apple")
set(WHATSON_ANDROID_DIR "${WHATSON_PLATFORM_DIR}/Android")
set(WHATSON_IOS_INFO_PLIST "${WHATSON_APPLE_DIR}/iOS/Info.plist")
set(WHATSON_DEFAULT_INFO_PLIST "${WHATSON_APPLE_DIR}/Info.plist")
set(WHATSON_ENTITLEMENTS "${WHATSON_APPLE_DIR}/WhatSon.entitlements")
set(WHATSON_APPLE_BUNDLE_ID "com.lvrs.whatson" CACHE STRING "Apple bundle id for WhatSon app")
set(WHATSON_ANDROID_PACKAGE_ID "com.lvrs.whatson" CACHE STRING "Android package id used by launch targets")

find_package(Qt6 6.5 REQUIRED COMPONENTS Core)
qt_standard_project_setup(REQUIRES 6.5)

message(STATUS "WhatSon configuration")
message(STATUS "  WHATSON_BUILD_APP: ${WHATSON_BUILD_APP}")
message(STATUS "  WHATSON_BUILD_DAEMON: ${WHATSON_BUILD_DAEMON}")
message(STATUS "  CMAKE_PREFIX_PATH: ${CMAKE_PREFIX_PATH}")
message(STATUS "  QT_ROOT_PATH: ${QT_ROOT_PATH}")
message(STATUS "  LVRS_PREFIX: ${LVRS_PREFIX}")
whatson_runtime_platform(_whatson_runtime_platform)
message(STATUS "  WHATSON_RUNTIME_PLATFORM: ${_whatson_runtime_platform}")

function(whatson_configure_app_target target_name)
    if (EXISTS "${WHATSON_ANDROID_DIR}")
        set_property(TARGET ${target_name} PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR "${WHATSON_ANDROID_DIR}")
    endif ()

    if (APPLE OR CMAKE_SYSTEM_NAME STREQUAL "iOS")
        if (EXISTS "${WHATSON_IOS_INFO_PLIST}")
            set_target_properties(${target_name} PROPERTIES
                    MACOSX_BUNDLE_INFO_PLIST "${WHATSON_IOS_INFO_PLIST}"
            )
        elseif (EXISTS "${WHATSON_DEFAULT_INFO_PLIST}")
            set_target_properties(${target_name} PROPERTIES
                    MACOSX_BUNDLE_INFO_PLIST "${WHATSON_DEFAULT_INFO_PLIST}"
            )
        endif ()

        if (EXISTS "${WHATSON_ENTITLEMENTS}")
            set_target_properties(${target_name} PROPERTIES
                    XCODE_ATTRIBUTE_CODE_SIGN_ENTITLEMENTS "${WHATSON_ENTITLEMENTS}"
            )
        endif ()
    endif ()

    if (CMAKE_SYSTEM_NAME STREQUAL "iOS")
        if (COMMAND qt_import_plugins)
            qt_import_plugins(${target_name} EXCLUDE_BY_TYPE qmltooling)
        endif ()
    endif ()
endfunction()

function(whatson_ensure_lvrs_plugin_target)
    if (TARGET LVRSCoreplugin)
        return()
    endif ()

    if (TARGET LVRS::LVRSCoreplugin)
        add_library(LVRSCoreplugin INTERFACE IMPORTED)
        set_target_properties(LVRSCoreplugin PROPERTIES
                INTERFACE_LINK_LIBRARIES "LVRS::LVRSCoreplugin"
        )
        return()
    endif ()

    if (TARGET LVRS::LVRS)
        add_library(LVRSCoreplugin INTERFACE IMPORTED)
        set_target_properties(LVRSCoreplugin PROPERTIES
                INTERFACE_LINK_LIBRARIES "LVRS::LVRS"
        )
    endif ()
endfunction()

if (WHATSON_BUILD_APP)
    add_subdirectory(src/app)
endif ()

if (WHATSON_BUILD_DAEMON)
    add_subdirectory(src/daemon)
endif ()

set(WHATSON_BUILD_TARGETS)
if (TARGET WhatSon)
    list(APPEND WHATSON_BUILD_TARGETS WhatSon)
endif ()
if (TARGET whats_on_daemon)
    list(APPEND WHATSON_BUILD_TARGETS whats_on_daemon)
endif ()

if (WHATSON_BUILD_TARGETS)
    add_custom_target(whatson_build_all DEPENDS ${WHATSON_BUILD_TARGETS})
endif ()

if (TARGET WhatSon)
    add_custom_target(whatson_run_app
            COMMAND "$<TARGET_FILE:WhatSon>"
            DEPENDS WhatSon
            USES_TERMINAL
            COMMENT "Run WhatSon app on current host"
    )
endif ()

if (TARGET whats_on_daemon)
    add_custom_target(whatson_run_daemon
            COMMAND "$<TARGET_FILE:whats_on_daemon>"
            DEPENDS whats_on_daemon
            USES_TERMINAL
            COMMENT "Run WhatSon daemon on current host"
    )

    add_custom_target(whatson_healthcheck_daemon
            COMMAND "$<TARGET_FILE:whats_on_daemon>" --healthcheck
            DEPENDS whats_on_daemon
            USES_TERMINAL
            COMMENT "Run daemon healthcheck"
    )
endif ()

if (TARGET WhatSon)
    find_program(WHATSON_XCRUN xcrun)
    if (WHATSON_XCRUN)
        add_custom_target(whatson_launch_ios
                COMMAND "${CMAKE_COMMAND}" --build "${CMAKE_BINARY_DIR}" --target bootstrap_WhatSon_ios
                COMMAND "${WHATSON_XCRUN}" simctl launch booted "${WHATSON_APPLE_BUNDLE_ID}"
                DEPENDS WhatSon
                USES_TERMINAL
                COMMENT "Launch WhatSon on iOS simulator"
        )

        add_custom_target(launch_WhatSon_ios
                DEPENDS whatson_launch_ios
        )
    endif ()

    find_program(WHATSON_ADB adb HINTS "$ENV{HOME}/Library/Android/sdk/platform-tools")
    if (WHATSON_ADB)
        add_custom_target(whatson_launch_android
                COMMAND "${CMAKE_COMMAND}" --build "${CMAKE_BINARY_DIR}" --target bootstrap_WhatSon_android
                COMMAND "${WHATSON_ADB}" shell monkey -p "${WHATSON_ANDROID_PACKAGE_ID}" -c android.intent.category.LAUNCHER 1
                DEPENDS WhatSon
                USES_TERMINAL
                COMMENT "Launch WhatSon on Android device/emulator"
        )

        add_custom_target(launch_WhatSon_android
                DEPENDS whatson_launch_android
        )
    endif ()
endif ()

set(WHATSON_DIST_DIR "${CMAKE_BINARY_DIR}/dist")
if (WHATSON_BUILD_TARGETS)
    add_custom_target(whatson_export_binaries
            COMMAND "${CMAKE_COMMAND}" -E rm -rf "${WHATSON_DIST_DIR}"
            COMMAND "${CMAKE_COMMAND}" -E make_directory "${WHATSON_DIST_DIR}"
            COMMENT "Export host binaries to ${WHATSON_DIST_DIR}"
    )

    if (TARGET WhatSon)
        if (APPLE)
            add_custom_command(TARGET whatson_export_binaries POST_BUILD
                    COMMAND "${CMAKE_COMMAND}" -E copy_directory
                    "$<TARGET_BUNDLE_DIR:WhatSon>"
                    "${WHATSON_DIST_DIR}/WhatSon.app"
            )
        else ()
            add_custom_command(TARGET whatson_export_binaries POST_BUILD
                    COMMAND "${CMAKE_COMMAND}" -E copy
                    "$<TARGET_FILE:WhatSon>"
                    "${WHATSON_DIST_DIR}/$<TARGET_FILE_NAME:WhatSon>"
            )
        endif ()
    endif ()

    if (TARGET whats_on_daemon)
        add_custom_command(TARGET whatson_export_binaries POST_BUILD
                COMMAND "${CMAKE_COMMAND}" -E copy
                "$<TARGET_FILE:whats_on_daemon>"
                "${WHATSON_DIST_DIR}/$<TARGET_FILE_NAME:whats_on_daemon>"
        )
    endif ()

    add_dependencies(whatson_export_binaries whatson_build_all)
endif ()

include(GNUInstallDirs)
if (TARGET WhatSon)
    install(TARGETS WhatSon
            BUNDLE DESTINATION .
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif ()
if (TARGET whats_on_daemon)
    install(TARGETS whats_on_daemon
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif ()

set(CPACK_PACKAGE_NAME "WhatSon")
set(CPACK_PACKAGE_VENDOR "LVRS")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
if (APPLE)
    set(CPACK_GENERATOR "DragNDrop;ZIP")
elseif (WIN32)
    set(CPACK_GENERATOR "ZIP")
elseif (UNIX)
    set(CPACK_GENERATOR "TGZ;ZIP")
endif ()
include(CPack)

if (CMAKE_CPACK_COMMAND)
    add_custom_target(whatson_package
            COMMAND "${CMAKE_CPACK_COMMAND}" --config "${CMAKE_BINARY_DIR}/CPackConfig.cmake"
            USES_TERMINAL
            COMMENT "Generate distributable package for current platform"
    )
    if (TARGET whatson_build_all)
        add_dependencies(whatson_package whatson_build_all)
    endif ()
endif ()
