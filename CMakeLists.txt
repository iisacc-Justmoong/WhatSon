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
set(WHATSON_ANDROID_PACKAGE_ID "com.lvrs.whatson" CACHE STRING "Android package id for WhatSon app")

find_package(Qt6 6.5 REQUIRED COMPONENTS Core)
qt_standard_project_setup(REQUIRES 6.5)

message(STATUS "WhatSon configuration")
message(STATUS "  WHATSON_BUILD_APP: ${WHATSON_BUILD_APP}")
message(STATUS "  WHATSON_BUILD_DAEMON: ${WHATSON_BUILD_DAEMON}")
message(STATUS "  CMAKE_PREFIX_PATH: ${CMAKE_PREFIX_PATH}")
message(STATUS "  QT_ROOT_PATH: ${QT_ROOT_PATH}")
message(STATUS "  LVRS_PREFIX: ${LVRS_PREFIX}")

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

if (TARGET launch_WhatSon_ios)
    add_custom_target(whatson_launch_ios DEPENDS launch_WhatSon_ios)
endif ()

if (TARGET launch_WhatSon_android)
    add_custom_target(whatson_launch_android DEPENDS launch_WhatSon_android)
endif ()

if (TARGET launch_WhatSon_wasm)
    add_custom_target(whatson_launch_wasm DEPENDS launch_WhatSon_wasm)
endif ()

if (TARGET export_WhatSon_xcodeproj)
    add_custom_target(whatson_export_xcodeproj DEPENDS export_WhatSon_xcodeproj)
endif ()

if (TARGET export_WhatSon_android_studio)
    add_custom_target(whatson_export_android_studio DEPENDS export_WhatSon_android_studio)
endif ()

if (TARGET export_WhatSon_wasm_site)
    add_custom_target(whatson_export_wasm_site DEPENDS export_WhatSon_wasm_site)
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
