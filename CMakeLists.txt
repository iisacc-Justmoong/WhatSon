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
endif ()

if (QT_ROOT_PATH)
    list(APPEND CMAKE_PREFIX_PATH "${QT_ROOT_PATH}")
endif ()

option(WHATSON_BUILD_APP "Build the Qt Quick desktop app" ON)
option(WHATSON_BUILD_DAEMON "Build the background daemon" ON)
option(WHATSON_BUILD_CLI "Build the Rust CLI launcher" ON)
option(WHATSON_ENABLE_TESTING "Enable automated tests for TDD workflow" ON)
option(WHATSON_RUN_TESTS_ON_BUILD "Run automated tests on every build" ON)
option(WHATSON_ENABLE_DEV_TOOLING "Enable developer lint/format convenience targets" ON)
get_filename_component(_whatson_binary_dir_name "${CMAKE_BINARY_DIR}" NAME)
set(_WHATSON_IS_TRIAL_BUILD_DEFAULT OFF)
if (_whatson_binary_dir_name STREQUAL "build-trial")
    set(_WHATSON_IS_TRIAL_BUILD_DEFAULT ON)
endif ()
option(WHATSON_IS_TRIAL_BUILD "Mark this build tree as the dedicated trial packaging tree." ${_WHATSON_IS_TRIAL_BUILD_DEFAULT})
set(_WHATSON_ENABLE_TRIAL_BUILD_MIRROR_DEFAULT ON)
if (WHATSON_IS_TRIAL_BUILD OR CMAKE_CROSSCOMPILING)
    set(_WHATSON_ENABLE_TRIAL_BUILD_MIRROR_DEFAULT OFF)
endif ()
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
if (WHATSON_ENABLE_TESTING)
    include(CTest)
endif ()

message(STATUS "WhatSon configuration")
message(STATUS "  WHATSON_BUILD_APP: ${WHATSON_BUILD_APP}")
message(STATUS "  WHATSON_BUILD_DAEMON: ${WHATSON_BUILD_DAEMON}")
message(STATUS "  WHATSON_BUILD_CLI: ${WHATSON_BUILD_CLI}")
message(STATUS "  BUILD_TESTING: ${BUILD_TESTING}")
message(STATUS "  WHATSON_RUN_TESTS_ON_BUILD: ${WHATSON_RUN_TESTS_ON_BUILD}")
message(STATUS "  WHATSON_ENABLE_DEV_TOOLING: ${WHATSON_ENABLE_DEV_TOOLING}")
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
    if (CMAKE_CROSSCOMPILING)
        message(STATUS "  Tests are disabled while cross-compiling.")
    else ()
        add_subdirectory(tests)
    endif ()
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
if (TARGET whatson_tests)
    list(APPEND WHATSON_BUILD_TARGETS whatson_tests)
endif ()

if (WHATSON_BUILD_TARGETS)
    add_custom_target(whatson_build_all DEPENDS ${WHATSON_BUILD_TARGETS})
endif ()

if (TARGET whatson_tests)
    set(_whatson_ctest_command
            "${CMAKE_CTEST_COMMAND}"
            "--output-on-failure"
            "--test-dir"
            "${CMAKE_BINARY_DIR}"
    )
    if (CMAKE_CONFIGURATION_TYPES)
        list(APPEND _whatson_ctest_command "-C" "$<CONFIG>")
    endif ()

    add_custom_target(whatson_test
            COMMAND ${_whatson_ctest_command}
            DEPENDS whatson_tests
            USES_TERMINAL
            COMMENT "Run WhatSon automated tests"
    )

    if (WHATSON_RUN_TESTS_ON_BUILD)
        add_custom_target(whatson_test_on_build ALL DEPENDS whatson_test)
    endif ()
endif ()

function(_whatson_add_missing_tool_target target_name tool_name install_hint)
    add_custom_target(${target_name}
            COMMAND "${CMAKE_COMMAND}" -E echo "${tool_name} was not found. ${install_hint}"
            COMMAND "${CMAKE_COMMAND}" -E false
            USES_TERMINAL
            COMMENT "Missing developer tool: ${tool_name}"
    )
endfunction()

if (WHATSON_ENABLE_DEV_TOOLING)
    if (APPLE)
        set(_whatson_host_qml_platform "macos")
    elseif (WIN32)
        set(_whatson_host_qml_platform "windows")
    else ()
        set(_whatson_host_qml_platform "linux")
    endif ()

    set(_whatson_qt_bin_hints)
    if (QT_ROOT_PATH AND EXISTS "${QT_ROOT_PATH}/bin")
        list(APPEND _whatson_qt_bin_hints "${QT_ROOT_PATH}/bin")
    endif ()

    find_program(WHATSON_QMLLINT_EXECUTABLE NAMES qmllint HINTS ${_whatson_qt_bin_hints})
    find_program(WHATSON_QMLFORMAT_EXECUTABLE NAMES qmlformat HINTS ${_whatson_qt_bin_hints})
    find_program(WHATSON_CLANG_TIDY_EXECUTABLE NAMES clang-tidy clang-tidy-18 clang-tidy-17 clang-tidy-16)

    message(STATUS "  WHATSON_QMLLINT_EXECUTABLE: ${WHATSON_QMLLINT_EXECUTABLE}")
    message(STATUS "  WHATSON_QMLFORMAT_EXECUTABLE: ${WHATSON_QMLFORMAT_EXECUTABLE}")
    message(STATUS "  WHATSON_CLANG_TIDY_EXECUTABLE: ${WHATSON_CLANG_TIDY_EXECUTABLE}")

    file(GLOB_RECURSE WHATSON_DEV_QML_FILES
            CONFIGURE_DEPENDS
            RELATIVE "${CMAKE_SOURCE_DIR}"
            "src/app/qml/*.qml"
    )

    file(GLOB_RECURSE WHATSON_DEV_CPP_FILES
            CONFIGURE_DEPENDS
            RELATIVE "${CMAKE_SOURCE_DIR}"
            "src/app/*.cpp"
            "src/app/*.cc"
            "src/app/*.cxx"
    )

    if (WHATSON_BUILD_DAEMON)
        file(GLOB_RECURSE WHATSON_DEV_DAEMON_CPP_FILES
                CONFIGURE_DEPENDS
                RELATIVE "${CMAKE_SOURCE_DIR}"
                "src/daemon/*.cpp"
                "src/daemon/*.cc"
                "src/daemon/*.cxx"
        )
        list(APPEND WHATSON_DEV_CPP_FILES ${WHATSON_DEV_DAEMON_CPP_FILES})
    endif ()

    if (BUILD_TESTING AND NOT CMAKE_CROSSCOMPILING)
        file(GLOB_RECURSE WHATSON_DEV_TEST_CPP_FILES
                CONFIGURE_DEPENDS
                RELATIVE "${CMAKE_SOURCE_DIR}"
                "tests/*.cpp"
                "tests/*.cc"
                "tests/*.cxx"
        )
        list(APPEND WHATSON_DEV_CPP_FILES ${WHATSON_DEV_TEST_CPP_FILES})
    endif ()

    list(REMOVE_DUPLICATES WHATSON_DEV_QML_FILES)
    list(REMOVE_DUPLICATES WHATSON_DEV_CPP_FILES)
    list(SORT WHATSON_DEV_QML_FILES)
    list(SORT WHATSON_DEV_CPP_FILES)

    set(WHATSON_DEV_QML_FILES_ABS ${WHATSON_DEV_QML_FILES})
    list(TRANSFORM WHATSON_DEV_QML_FILES_ABS PREPEND "${CMAKE_SOURCE_DIR}/")

    set(WHATSON_DEV_CPP_FILES_ABS ${WHATSON_DEV_CPP_FILES})
    list(TRANSFORM WHATSON_DEV_CPP_FILES_ABS PREPEND "${CMAKE_SOURCE_DIR}/")

    set(WHATSON_DEV_QML_FILELIST "${CMAKE_BINARY_DIR}/whatson_qml_files.txt")
    string(REPLACE ";" "\n" _whatson_dev_qml_filelist_text "${WHATSON_DEV_QML_FILES_ABS}")
    file(GENERATE OUTPUT "${WHATSON_DEV_QML_FILELIST}" CONTENT "${_whatson_dev_qml_filelist_text}\n")

    set(WHATSON_DEV_CPP_FILELIST "${CMAKE_BINARY_DIR}/whatson_cpp_files.txt")
    string(REPLACE ";" "\n" _whatson_dev_cpp_filelist_text "${WHATSON_DEV_CPP_FILES_ABS}")
    file(GENERATE OUTPUT "${WHATSON_DEV_CPP_FILELIST}" CONTENT "${_whatson_dev_cpp_filelist_text}\n")

    set(_whatson_qml_import_dirs)
    if (LVRS_PREFIX AND EXISTS "${LVRS_PREFIX}/platforms/${_whatson_host_qml_platform}/lib/qt6/qml")
        list(APPEND _whatson_qml_import_dirs "${LVRS_PREFIX}/platforms/${_whatson_host_qml_platform}/lib/qt6/qml")
    endif ()
    if (QT_ROOT_PATH AND EXISTS "${QT_ROOT_PATH}/qml")
        list(APPEND _whatson_qml_import_dirs "${QT_ROOT_PATH}/qml")
    endif ()
    if (EXISTS "${CMAKE_BINARY_DIR}/src/app")
        list(APPEND _whatson_qml_import_dirs "${CMAKE_BINARY_DIR}/src/app")
    endif ()
    list(REMOVE_DUPLICATES _whatson_qml_import_dirs)

    if (WHATSON_QMLLINT_EXECUTABLE)
        set(_whatson_qmllint_args "--max-warnings" "-1")
        foreach (_whatson_qml_import_dir IN LISTS _whatson_qml_import_dirs)
            list(APPEND _whatson_qmllint_args "-I" "${_whatson_qml_import_dir}")
        endforeach ()
        if (EXISTS "${CMAKE_BINARY_DIR}/src/app/WhatSon/App/qmldir")
            list(APPEND _whatson_qmllint_args "-i" "${CMAKE_BINARY_DIR}/src/app/WhatSon/App/qmldir")
        endif ()
        list(APPEND _whatson_qmllint_args ${WHATSON_DEV_QML_FILES_ABS})

        add_custom_target(whatson_qmllint
                COMMAND "${WHATSON_QMLLINT_EXECUTABLE}" ${_whatson_qmllint_args}
                USES_TERMINAL
                COMMENT "Run qmllint on WhatSon QML sources"
        )
    else ()
        _whatson_add_missing_tool_target(
                whatson_qmllint
                "qmllint"
                "Install Qt QML tools or set QT_ROOT_PATH to a Qt prefix that contains qmllint.")
    endif ()

    if (WHATSON_QMLFORMAT_EXECUTABLE)
        add_custom_target(whatson_qmlformat_check
                COMMAND "${CMAKE_COMMAND}"
                "-DQMLFORMAT_EXECUTABLE=${WHATSON_QMLFORMAT_EXECUTABLE}"
                "-DQML_FILELIST=${WHATSON_DEV_QML_FILELIST}"
                -P "${CMAKE_SOURCE_DIR}/cmake/RunWhatSonQmlFormatCheck.cmake"
                USES_TERMINAL
                COMMENT "Check QML formatting with qmlformat"
        )

        add_custom_target(whatson_qmlformat_fix
                COMMAND "${WHATSON_QMLFORMAT_EXECUTABLE}" --inplace --files "${WHATSON_DEV_QML_FILELIST}"
                USES_TERMINAL
                COMMENT "Rewrite WhatSon QML sources in-place with qmlformat"
        )
    else ()
        _whatson_add_missing_tool_target(
                whatson_qmlformat_check
                "qmlformat"
                "Install Qt QML tools or set QT_ROOT_PATH to a Qt prefix that contains qmlformat.")
        _whatson_add_missing_tool_target(
                whatson_qmlformat_fix
                "qmlformat"
                "Install Qt QML tools or set QT_ROOT_PATH to a Qt prefix that contains qmlformat.")
    endif ()

    if (WHATSON_CLANG_TIDY_EXECUTABLE)
        add_custom_target(whatson_clang_tidy
                COMMAND "${CMAKE_COMMAND}"
                "-DCLANG_TIDY_EXECUTABLE=${WHATSON_CLANG_TIDY_EXECUTABLE}"
                "-DCPP_FILELIST=${WHATSON_DEV_CPP_FILELIST}"
                "-DBUILD_DIR=${CMAKE_BINARY_DIR}"
                -P "${CMAKE_SOURCE_DIR}/cmake/RunWhatSonClangTidy.cmake"
                USES_TERMINAL
                COMMENT "Run clang-tidy on WhatSon C++ sources"
        )
    else ()
        _whatson_add_missing_tool_target(
                whatson_clang_tidy
                "clang-tidy"
                "Install clang-tidy and re-run CMake configure so the target can bind to the executable.")
    endif ()

    add_custom_target(whatson_dev_checks
            DEPENDS
            whatson_qmllint
            whatson_qmlformat_check
            COMMENT "Run the default WhatSon developer lint checks"
    )
    if (WHATSON_CLANG_TIDY_EXECUTABLE)
        add_dependencies(whatson_dev_checks whatson_clang_tidy)
    endif ()
endif ()

if (WHATSON_ENABLE_IOS_XCODEPROJ_ON_BUILD AND APPLE AND NOT CMAKE_SYSTEM_NAME STREQUAL "iOS" AND TARGET WhatSon)
    set(_whatson_qt_ios_prefix "")
    if (DEFINED WHATSON_QT_IOS_PREFIX AND NOT WHATSON_QT_IOS_PREFIX STREQUAL "")
        set(_whatson_qt_ios_prefix "${WHATSON_QT_IOS_PREFIX}")
    elseif (DEFINED ENV{QT_IOS_PREFIX} AND NOT "$ENV{QT_IOS_PREFIX}" STREQUAL "")
        set(_whatson_qt_ios_prefix "$ENV{QT_IOS_PREFIX}")
    elseif (QT_ROOT_PATH AND EXISTS "${QT_ROOT_PATH}")
        get_filename_component(_whatson_qt_root_parent "${QT_ROOT_PATH}" DIRECTORY)
        if (EXISTS "${_whatson_qt_root_parent}/ios")
            set(_whatson_qt_ios_prefix "${_whatson_qt_root_parent}/ios")
        endif ()
        unset(_whatson_qt_root_parent)
    endif ()
    if (NOT _whatson_qt_ios_prefix STREQUAL "")
        set(WHATSON_QT_IOS_PREFIX "${_whatson_qt_ios_prefix}" CACHE PATH
                "Qt iOS prefix used for iOS Xcode project generation.")
    endif ()

    set(_whatson_lvrs_ios_prefix "")
    if (DEFINED WHATSON_LVRS_IOS_PREFIX AND NOT WHATSON_LVRS_IOS_PREFIX STREQUAL "")
        set(_whatson_lvrs_ios_prefix "${WHATSON_LVRS_IOS_PREFIX}")
    elseif (LVRS_PREFIX AND EXISTS "${LVRS_PREFIX}/platforms/ios")
        set(_whatson_lvrs_ios_prefix "${LVRS_PREFIX}/platforms/ios")
    elseif (DEFINED ENV{LVRS_IOS_PREFIX} AND NOT "$ENV{LVRS_IOS_PREFIX}" STREQUAL "")
        set(_whatson_lvrs_ios_prefix "$ENV{LVRS_IOS_PREFIX}")
    endif ()
    if (NOT _whatson_lvrs_ios_prefix STREQUAL "")
        set(WHATSON_LVRS_IOS_PREFIX "${_whatson_lvrs_ios_prefix}" CACHE PATH
                "LVRS iOS prefix used for iOS Xcode project generation.")
    endif ()

    if (NOT DEFINED WHATSON_QT_IOS_PREFIX OR WHATSON_QT_IOS_PREFIX STREQUAL "")
        message(WARNING "WHATSON_ENABLE_IOS_XCODEPROJ_ON_BUILD is ON but WHATSON_QT_IOS_PREFIX could not be resolved.")
    else ()
        set(_whatson_ios_toolchain "${WHATSON_QT_IOS_PREFIX}/lib/cmake/Qt6/qt.toolchain.cmake")
        if (NOT EXISTS "${_whatson_ios_toolchain}")
            message(WARNING "Qt iOS toolchain file was not found: ${_whatson_ios_toolchain}")
        else ()
            # Qt's optional gRPC / Protobuf Quick plugin packages try to resolve host protoc during
            # cross-compiling even though WhatSon does not use those modules.
            set(_whatson_ios_optional_qml_package_flags
                    -DCMAKE_DISABLE_FIND_PACKAGE_Qt6GrpcQuick=TRUE
                    -DCMAKE_DISABLE_FIND_PACKAGE_Qt6ProtobufQuick=TRUE
            )
            set(_whatson_ios_configure_cmd
                    "${CMAKE_COMMAND}"
                    -G Xcode
                    -S "${CMAKE_SOURCE_DIR}"
                    -B "${WHATSON_IOS_XCODEPROJ_DIR}"
                    -DCMAKE_SYSTEM_NAME=iOS
                    "-DCMAKE_TOOLCHAIN_FILE=${_whatson_ios_toolchain}"
                    -DCMAKE_BUILD_TYPE=Release
                    -DCMAKE_OSX_SYSROOT=iphonesimulator
                    "-DCMAKE_OSX_ARCHITECTURES=${WHATSON_IOS_SIMULATOR_ARCH}"
                    -DCMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_ALLOWED=NO
                    -DCMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED=NO
                    "-DCMAKE_XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER=${WHATSON_APPLE_BUNDLE_ID}"
                    "-DWHATSON_PLATFORM_DIR=${WHATSON_PLATFORM_DIR}"
                    "-DWHATSON_APPLE_DIR=${WHATSON_APPLE_DIR}"
                    "-DWHATSON_IOS_INFO_PLIST=${WHATSON_IOS_INFO_PLIST}"
                    "-DWHATSON_DEFAULT_INFO_PLIST=${WHATSON_DEFAULT_INFO_PLIST}"
                    "-DWHATSON_ENTITLEMENTS=${WHATSON_ENTITLEMENTS}"
                    "-DWHATSON_APPLE_BUNDLE_ID=${WHATSON_APPLE_BUNDLE_ID}"
                    "-DWHATSON_ANDROID_PACKAGE_ID=${WHATSON_ANDROID_PACKAGE_ID}"
                    ${_whatson_ios_optional_qml_package_flags}
            )

            if (DEFINED WHATSON_LVRS_IOS_PREFIX AND EXISTS "${WHATSON_LVRS_IOS_PREFIX}/lib/cmake/LVRS")
                list(APPEND _whatson_ios_configure_cmd
                        "-DLVRS_DIR=${WHATSON_LVRS_IOS_PREFIX}/lib/cmake/LVRS")
            endif ()

            set(_whatson_prepare_ios_export_script
                    "${CMAKE_BINARY_DIR}/CMakeFiles/PrepareWhatSonIosXcodeproj.cmake")
            configure_file(
                    "${CMAKE_SOURCE_DIR}/cmake/PrepareWhatSonIosXcodeproj.cmake.in"
                    "${_whatson_prepare_ios_export_script}"
                    @ONLY
            )

            add_custom_target(whatson_generate_ios_xcodeproj
                    COMMAND "${CMAKE_COMMAND}" -P "${_whatson_prepare_ios_export_script}"
                    COMMAND ${_whatson_ios_configure_cmd}
                    BYPRODUCTS "${WHATSON_IOS_XCODEPROJ_DIR}/WhatSon.xcodeproj"
                    USES_TERMINAL
                    COMMENT "Generate iOS Xcode project from platform/Apple metadata"
            )
        endif ()
    endif ()
endif ()

if (TARGET WhatSon)
    add_custom_target(whatson_run_app
            COMMAND "$<TARGET_FILE:WhatSon>"
            DEPENDS WhatSon
            USES_TERMINAL
            COMMENT "Run WhatSon app on current host"
    )
endif ()

if (TARGET whatson_cli AND DEFINED WHATSON_CLI_BINARY_PATH AND NOT WHATSON_CLI_BINARY_PATH STREQUAL "")
    add_custom_target(whatson_run_cli
            COMMAND "${WHATSON_CLI_BINARY_PATH}"
            DEPENDS whatson_cli
            USES_TERMINAL
            COMMENT "Run Rust CLI launcher (whatson)"
    )
endif ()

if (TARGET WhatSon_daemon)
    add_custom_target(whatson_run_daemon
            COMMAND "$<TARGET_FILE:WhatSon_daemon>"
            DEPENDS WhatSon_daemon
            USES_TERMINAL
            COMMENT "Run WhatSon daemon on current host"
    )

    add_custom_target(whatson_healthcheck_daemon
            COMMAND "$<TARGET_FILE:WhatSon_daemon>" --healthcheck
            DEPENDS WhatSon_daemon
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

if (TARGET whatson_generate_ios_xcodeproj)
    add_custom_target(whatson_export_xcodeproj DEPENDS whatson_generate_ios_xcodeproj)
elseif (TARGET export_WhatSon_xcodeproj)
    add_custom_target(whatson_export_xcodeproj DEPENDS export_WhatSon_xcodeproj)
endif ()

if (TARGET export_WhatSon_android_studio)
    add_custom_target(whatson_export_android_studio DEPENDS export_WhatSon_android_studio)
endif ()

if (TARGET export_WhatSon_wasm_site)
    add_custom_target(whatson_export_wasm_site DEPENDS export_WhatSon_wasm_site)
endif ()

set(WHATSON_DIST_DIR "${CMAKE_BINARY_DIR}/dist")
set(_whatson_native_desktop_install_tree FALSE)
if (NOT CMAKE_CROSSCOMPILING)
    if (WIN32)
        set(_whatson_native_desktop_install_tree TRUE)
    elseif (APPLE AND NOT CMAKE_SYSTEM_NAME STREQUAL "iOS")
        set(_whatson_native_desktop_install_tree TRUE)
    elseif (UNIX AND NOT APPLE)
        set(_whatson_native_desktop_install_tree TRUE)
    endif ()
endif ()

if (WHATSON_BUILD_TARGETS)
    if (_whatson_native_desktop_install_tree)
        set(_whatson_install_dist_command
                "${CMAKE_COMMAND}"
                --install "${CMAKE_BINARY_DIR}"
                --prefix "${WHATSON_DIST_DIR}")
        if (CMAKE_CONFIGURATION_TYPES)
            list(APPEND _whatson_install_dist_command --config "$<CONFIG>")
        endif ()

        add_custom_target(whatson_export_binaries
                COMMAND "${CMAKE_COMMAND}" -E rm -rf "${WHATSON_DIST_DIR}"
                COMMAND "${CMAKE_COMMAND}" -E make_directory "${WHATSON_DIST_DIR}"
                COMMAND ${_whatson_install_dist_command}
                COMMENT "Export self-contained host install tree to ${WHATSON_DIST_DIR}"
        )
    else ()
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

        if (TARGET WhatSon_daemon)
            add_custom_command(TARGET whatson_export_binaries POST_BUILD
                    COMMAND "${CMAKE_COMMAND}" -E copy
                    "$<TARGET_FILE:WhatSon_daemon>"
                    "${WHATSON_DIST_DIR}/$<TARGET_FILE_NAME:WhatSon_daemon>"
            )
        endif ()

        if (TARGET whatson_cli AND DEFINED WHATSON_CLI_BINARY_PATH AND NOT WHATSON_CLI_BINARY_PATH STREQUAL "")
            add_custom_command(TARGET whatson_export_binaries POST_BUILD
                    COMMAND "${CMAKE_COMMAND}" -E copy
                    "${WHATSON_CLI_BINARY_PATH}"
                    "${WHATSON_DIST_DIR}/whatson${CMAKE_EXECUTABLE_SUFFIX}"
            )
        endif ()
    endif ()

    add_dependencies(whatson_export_binaries whatson_build_all)
endif ()

include(GNUInstallDirs)
set(WHATSON_LINUX_DESKTOP_FILE "")
if (UNIX AND NOT APPLE AND EXISTS "${WHATSON_LINUX_DIR}/WhatSon.desktop.in")
    set(WHATSON_LINUX_DESKTOP_FILE "${CMAKE_CURRENT_BINARY_DIR}/WhatSon.desktop")
    configure_file("${WHATSON_LINUX_DIR}/WhatSon.desktop.in" "${WHATSON_LINUX_DESKTOP_FILE}" @ONLY)
endif ()

if (TARGET WhatSon)
    install(TARGETS WhatSon
            BUNDLE DESTINATION .
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif ()
if (TARGET WhatSon_daemon)
    if (APPLE)
        install(TARGETS WhatSon_daemon
                BUNDLE DESTINATION .
                LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
                RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        )
    else ()
        install(TARGETS WhatSon_daemon
                LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
                RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        )
    endif ()
endif ()

if (TARGET whatson_cli AND DEFINED WHATSON_CLI_BINARY_PATH AND NOT WHATSON_CLI_BINARY_PATH STREQUAL "")
    install(PROGRAMS "${WHATSON_CLI_BINARY_PATH}" DESTINATION ${CMAKE_INSTALL_BINDIR})
endif ()

if (TARGET WhatSon AND UNIX AND NOT APPLE)
    if (NOT WHATSON_LINUX_DESKTOP_FILE STREQUAL "")
        install(FILES "${WHATSON_LINUX_DESKTOP_FILE}" DESTINATION "${CMAKE_INSTALL_DATADIR}/applications")
    endif ()

    if (EXISTS "${WHATSON_APP_ICON_PNG}")
        install(FILES "${WHATSON_APP_ICON_PNG}"
                DESTINATION "${CMAKE_INSTALL_DATADIR}/icons/hicolor/512x512/apps"
                RENAME "whatson.png")
        install(FILES "${WHATSON_APP_ICON_PNG}"
                DESTINATION "${CMAKE_INSTALL_DATADIR}/pixmaps"
                RENAME "whatson.png")
    endif ()
endif ()

if (TARGET WhatSon
        AND _whatson_native_desktop_install_tree
        AND COMMAND qt_generate_deploy_qml_app_script)
    qt_generate_deploy_qml_app_script(
            TARGET WhatSon
            OUTPUT_SCRIPT WHATSON_QT_DEPLOY_APP_SCRIPT
            NO_UNSUPPORTED_PLATFORM_ERROR
    )
    install(SCRIPT "${WHATSON_QT_DEPLOY_APP_SCRIPT}")
endif ()

set(_whatson_native_desktop_runtime_deploy FALSE)
if (NOT CMAKE_CROSSCOMPILING)
    if (WIN32)
        set(_whatson_native_desktop_runtime_deploy TRUE)
    elseif (UNIX AND NOT APPLE)
        set(_whatson_native_desktop_runtime_deploy TRUE)
    endif ()
endif ()

if (TARGET WhatSon_daemon
        AND _whatson_native_desktop_runtime_deploy
        AND COMMAND qt_generate_deploy_script)
    set(_whatson_daemon_qt_conf_arg "")
    if (WIN32)
        set(_whatson_daemon_qt_conf_arg "    GENERATE_QT_CONF\n")
    endif ()

    qt_generate_deploy_script(
            TARGET WhatSon_daemon
            OUTPUT_SCRIPT WHATSON_QT_DEPLOY_DAEMON_SCRIPT
            CONTENT "
qt_deploy_runtime_dependencies(
    EXECUTABLE \$<TARGET_FILE:WhatSon_daemon>
${_whatson_daemon_qt_conf_arg}\
    NO_TRANSLATIONS
)
")
    install(SCRIPT "${WHATSON_QT_DEPLOY_DAEMON_SCRIPT}")
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

if (TARGET whatson_package
        AND TARGET WhatSon
        AND NOT CMAKE_CROSSCOMPILING
        AND NOT WHATSON_IS_TRIAL_BUILD
        AND WHATSON_ENABLE_TRIAL_BUILD_MIRROR)
    set(_whatson_trial_cache_script "${CMAKE_BINARY_DIR}/CMakeFiles/whatson_trial_cache_init.cmake")
    set(_whatson_trial_cache_lines "")
    if (DEFINED CMAKE_PREFIX_PATH AND NOT CMAKE_PREFIX_PATH STREQUAL "")
        string(APPEND _whatson_trial_cache_lines
                "set(CMAKE_PREFIX_PATH [==[${CMAKE_PREFIX_PATH}]==] CACHE STRING \"Forwarded host prefix path list for the trial mirror.\" FORCE)\n")
    endif ()
    if (APPLE
            AND DEFINED WHATSON_TRIAL_BUILD_OSX_ARCHITECTURES
            AND NOT WHATSON_TRIAL_BUILD_OSX_ARCHITECTURES STREQUAL "")
        string(APPEND _whatson_trial_cache_lines
                "set(CMAKE_OSX_ARCHITECTURES [==[${WHATSON_TRIAL_BUILD_OSX_ARCHITECTURES}]==] CACHE STRING \"Forwarded macOS architecture list for the trial mirror.\" FORCE)\n")
    endif ()
    if (NOT _whatson_trial_cache_lines STREQUAL "")
        file(CONFIGURE OUTPUT "${_whatson_trial_cache_script}" CONTENT "${_whatson_trial_cache_lines}" @ONLY)
    endif ()

    set(_whatson_trial_configure_command
            "${CMAKE_COMMAND}")
    if (NOT _whatson_trial_cache_lines STREQUAL "")
        list(APPEND _whatson_trial_configure_command -C "${_whatson_trial_cache_script}")
    endif ()
    list(APPEND _whatson_trial_configure_command
            -G "${CMAKE_GENERATOR}")
    if (CMAKE_GENERATOR_PLATFORM)
        list(APPEND _whatson_trial_configure_command -A "${CMAKE_GENERATOR_PLATFORM}")
    endif ()
    if (CMAKE_GENERATOR_TOOLSET)
        list(APPEND _whatson_trial_configure_command -T "${CMAKE_GENERATOR_TOOLSET}")
    endif ()
    if (CMAKE_GENERATOR_INSTANCE)
        list(APPEND _whatson_trial_configure_command
                "-DCMAKE_GENERATOR_INSTANCE=${CMAKE_GENERATOR_INSTANCE}")
    endif ()

    list(APPEND _whatson_trial_configure_command
            -S "${CMAKE_SOURCE_DIR}"
            -B "${WHATSON_TRIAL_BUILD_DIR}"
            "-DWHATSON_IS_TRIAL_BUILD=ON"
            "-DWHATSON_ENABLE_TRIAL_BUILD_MIRROR=OFF"
            "-DWHATSON_TRIAL_BUILD_DIR=${WHATSON_TRIAL_BUILD_DIR}"
            "-DWHATSON_ENABLE_IOS_XCODEPROJ_ON_BUILD=OFF"
            "-DWHATSON_BUILD_APP=${WHATSON_BUILD_APP}"
            "-DWHATSON_BUILD_DAEMON=${WHATSON_BUILD_DAEMON}"
            "-DWHATSON_BUILD_CLI=${WHATSON_BUILD_CLI}"
            "-DWHATSON_ENABLE_TESTING=OFF"
            "-DWHATSON_RUN_TESTS_ON_BUILD=OFF"
            "-DWHATSON_ENABLE_DEV_TOOLING=OFF"
            "-DWHATSON_IOS_SIMULATOR_ARCH=${WHATSON_IOS_SIMULATOR_ARCH}")

    if (DEFINED CMAKE_BUILD_TYPE AND NOT CMAKE_BUILD_TYPE STREQUAL "")
        list(APPEND _whatson_trial_configure_command "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}")
    endif ()
    if (DEFINED CMAKE_MAKE_PROGRAM AND NOT CMAKE_MAKE_PROGRAM STREQUAL "")
        list(APPEND _whatson_trial_configure_command "-DCMAKE_MAKE_PROGRAM=${CMAKE_MAKE_PROGRAM}")
    endif ()
    if (DEFINED QT_ROOT_PATH AND NOT QT_ROOT_PATH STREQUAL "")
        list(APPEND _whatson_trial_configure_command "-DQT_ROOT_PATH=${QT_ROOT_PATH}")
    endif ()
    if (DEFINED LVRS_PREFIX AND NOT LVRS_PREFIX STREQUAL "")
        list(APPEND _whatson_trial_configure_command "-DLVRS_PREFIX=${LVRS_PREFIX}")
    endif ()
    if (DEFINED LVRS_DIR AND NOT LVRS_DIR STREQUAL "")
        list(APPEND _whatson_trial_configure_command "-DLVRS_DIR=${LVRS_DIR}")
    endif ()
    if (APPLE)
        if (DEFINED CMAKE_OSX_DEPLOYMENT_TARGET AND NOT CMAKE_OSX_DEPLOYMENT_TARGET STREQUAL "")
            list(APPEND _whatson_trial_configure_command
                    "-DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET}")
        endif ()
        if (DEFINED CMAKE_OSX_SYSROOT AND NOT CMAKE_OSX_SYSROOT STREQUAL "")
            list(APPEND _whatson_trial_configure_command "-DCMAKE_OSX_SYSROOT=${CMAKE_OSX_SYSROOT}")
        endif ()
    endif ()

    set(_whatson_trial_build_command
            "${CMAKE_COMMAND}"
            --build "${WHATSON_TRIAL_BUILD_DIR}"
            --target whatson_package)
    if (CMAKE_CONFIGURATION_TYPES)
        list(APPEND _whatson_trial_build_command --config "$<CONFIG>")
    endif ()

    add_custom_target(whatson_sync_trial_build
            COMMAND "${CMAKE_COMMAND}" -E make_directory "${WHATSON_TRIAL_BUILD_DIR}"
            COMMAND ${_whatson_trial_configure_command}
            COMMAND ${_whatson_trial_build_command}
            USES_TERMINAL
            COMMENT "Configure and build the dedicated build-trial packaging tree"
    )

    if (TARGET whatson_build_all)
        add_dependencies(whatson_build_all whatson_sync_trial_build)
    endif ()

    if (TARGET WhatSon)
        add_dependencies(WhatSon whatson_sync_trial_build)
    endif ()

    if (TARGET WhatSon_daemon)
        add_dependencies(WhatSon_daemon whatson_sync_trial_build)
    endif ()

    if (TARGET whatson_cli)
        add_dependencies(whatson_cli whatson_sync_trial_build)
    endif ()

    add_custom_target(whatson_sync_trial_build_on_build ALL
            DEPENDS whatson_sync_trial_build
    )
endif ()
