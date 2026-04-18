find_package(Qt6 6.5 REQUIRED COMPONENTS Quick QuickControls2 QuickDialogs2 Pdf PdfQuick)
find_package(LVRS CONFIG REQUIRED)
qt_policy(SET QTP0004 NEW)

set(WHATSON_APP_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

function(whatson_prune_missing_imported_includes target_name)
    if (NOT TARGET "${target_name}")
        return()
    endif ()

    get_target_property(_whatson_imported_includes "${target_name}" INTERFACE_INCLUDE_DIRECTORIES)
    if (NOT _whatson_imported_includes)
        return()
    endif ()

    set(_whatson_imported_includes_filtered)
    set(_whatson_imported_includes_pruned FALSE)
    foreach (_whatson_imported_include IN LISTS _whatson_imported_includes)
        if (EXISTS "${_whatson_imported_include}")
            list(APPEND _whatson_imported_includes_filtered "${_whatson_imported_include}")
        else ()
            set(_whatson_imported_includes_pruned TRUE)
        endif ()
    endforeach ()

    if (_whatson_imported_includes_pruned)
        set_property(TARGET "${target_name}" PROPERTY
                INTERFACE_INCLUDE_DIRECTORIES "${_whatson_imported_includes_filtered}")
    endif ()
endfunction()

whatson_prune_missing_imported_includes(Qt6::Pdf)
whatson_prune_missing_imported_includes(Qt6::PdfQuick)
whatson_prune_missing_imported_includes(Qt6::PdfPrivate)
whatson_prune_missing_imported_includes(Qt6::PdfQuickPrivate)

if (APPLE AND TARGET WrapOpenGL::WrapOpenGL)
    get_target_property(_whatson_wrap_opengl_links WrapOpenGL::WrapOpenGL INTERFACE_LINK_LIBRARIES)
    if (_whatson_wrap_opengl_links)
        set(_whatson_wrap_opengl_links_filtered)
        foreach (_whatson_wrap_link IN LISTS _whatson_wrap_opengl_links)
            if (_whatson_wrap_link STREQUAL "-framework AGL" OR _whatson_wrap_link MATCHES "AGL\\.framework")
                continue()
            endif ()
            list(APPEND _whatson_wrap_opengl_links_filtered "${_whatson_wrap_link}")
        endforeach ()
        set_property(TARGET WrapOpenGL::WrapOpenGL PROPERTY
                INTERFACE_LINK_LIBRARIES "${_whatson_wrap_opengl_links_filtered}")
    endif ()
endif ()

set_property(GLOBAL PROPERTY WHATSON_APP_QML_FILES "")
set_property(GLOBAL PROPERTY WHATSON_APP_QML_RESOURCE_FILES "")

function(whatson_app_register_sources)
    foreach (_whatson_source IN LISTS ARGN)
        if (IS_ABSOLUTE "${_whatson_source}")
            set(_whatson_abs_source "${_whatson_source}")
        else ()
            set(_whatson_abs_source "${CMAKE_CURRENT_SOURCE_DIR}/${_whatson_source}")
        endif ()

        if (NOT EXISTS "${_whatson_abs_source}")
            message(FATAL_ERROR "WhatSon app source was not found: ${_whatson_abs_source}")
        endif ()

        target_sources(WhatSon PRIVATE "${_whatson_abs_source}")
    endforeach ()
endfunction()

function(whatson_app_register_directory_sources base_dir)
    set(options RECURSE)
    set(multiValueArgs PATTERNS EXCLUDE_REGEX)
    cmake_parse_arguments(ARG "${options}" "" "${multiValueArgs}" ${ARGN})

    if (NOT IS_DIRECTORY "${base_dir}")
        message(FATAL_ERROR "WhatSon app source directory was not found: ${base_dir}")
    endif ()

    if (NOT ARG_PATTERNS)
        set(ARG_PATTERNS
                "*.c"
                "*.cc"
                "*.cpp"
                "*.cxx"
                "*.h"
                "*.hh"
                "*.hpp"
                "*.hxx")
    endif ()

    if (ARG_RECURSE)
        file(GLOB_RECURSE _whatson_directory_sources
                CONFIGURE_DEPENDS
                RELATIVE "${base_dir}"
                ${ARG_PATTERNS})
    else ()
        file(GLOB _whatson_directory_sources
                CONFIGURE_DEPENDS
                RELATIVE "${base_dir}"
                ${ARG_PATTERNS})
    endif ()

    if (NOT _whatson_directory_sources)
        return()
    endif ()

    list(SORT _whatson_directory_sources)
    foreach (_whatson_exclude_regex IN LISTS ARG_EXCLUDE_REGEX)
        list(FILTER _whatson_directory_sources EXCLUDE REGEX "${_whatson_exclude_regex}")
    endforeach ()

    if (NOT _whatson_directory_sources)
        return()
    endif ()

    list(TRANSFORM _whatson_directory_sources PREPEND "${base_dir}/")
    whatson_app_register_sources(${_whatson_directory_sources})
endfunction()

function(whatson_app_register_directory_include_directories base_dir)
    if (NOT IS_DIRECTORY "${base_dir}")
        message(FATAL_ERROR "WhatSon app include directory was not found: ${base_dir}")
    endif ()

    file(GLOB_RECURSE _whatson_header_files
            CONFIGURE_DEPENDS
            RELATIVE "${base_dir}"
            "*.h"
            "*.hh"
            "*.hpp"
            "*.hxx")

    set(_whatson_include_dirs "${base_dir}")
    foreach (_whatson_header_file IN LISTS _whatson_header_files)
        get_filename_component(_whatson_header_dir "${_whatson_header_file}" DIRECTORY)
        if (_whatson_header_dir STREQUAL "")
            continue()
        endif ()

        set(_whatson_include_dir_cursor "${_whatson_header_dir}")
        while (NOT _whatson_include_dir_cursor STREQUAL "")
            list(APPEND _whatson_include_dirs "${base_dir}/${_whatson_include_dir_cursor}")
            get_filename_component(_whatson_include_dir_cursor "${_whatson_include_dir_cursor}" DIRECTORY)
            if (_whatson_include_dir_cursor STREQUAL ".")
                set(_whatson_include_dir_cursor "")
            endif ()
        endwhile ()
    endforeach ()

    list(REMOVE_DUPLICATES _whatson_include_dirs)
    list(SORT _whatson_include_dirs)
    target_include_directories(WhatSon PRIVATE ${_whatson_include_dirs})
endfunction()

function(whatson_app_register_qml_entries)
    set(multiValueArgs QML_FILES RESOURCE_FILES)
    cmake_parse_arguments(ARG "" "" "${multiValueArgs}" ${ARGN})

    if (ARG_QML_FILES)
        set_property(GLOBAL APPEND PROPERTY WHATSON_APP_QML_FILES ${ARG_QML_FILES})
    endif ()
    if (ARG_RESOURCE_FILES)
        set_property(GLOBAL APPEND PROPERTY WHATSON_APP_QML_RESOURCE_FILES ${ARG_RESOURCE_FILES})
    endif ()
endfunction()

function(whatson_app_register_directory_qml base_dir)
    if (NOT IS_DIRECTORY "${base_dir}")
        message(FATAL_ERROR "WhatSon app QML directory was not found: ${base_dir}")
    endif ()

    file(GLOB_RECURSE _whatson_qml_files
            CONFIGURE_DEPENDS
            RELATIVE "${WHATSON_APP_SOURCE_DIR}"
            "${base_dir}/*.qml")
    file(GLOB_RECURSE _whatson_qml_resource_files
            CONFIGURE_DEPENDS
            RELATIVE "${WHATSON_APP_SOURCE_DIR}"
            "${base_dir}/*.js")

    if (_whatson_qml_files)
        list(SORT _whatson_qml_files)
    endif ()
    if (_whatson_qml_resource_files)
        list(SORT _whatson_qml_resource_files)
    endif ()

    whatson_app_register_qml_entries(
            QML_FILES ${_whatson_qml_files}
            RESOURCE_FILES ${_whatson_qml_resource_files})
endfunction()

qt_add_executable(WhatSon main.cpp)
target_include_directories(WhatSon PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
whatson_app_register_directory_include_directories("${CMAKE_CURRENT_SOURCE_DIR}/models")

if (WHATSON_IS_TRIAL_BUILD AND NOT ANDROID AND NOT (APPLE AND CMAKE_SYSTEM_NAME STREQUAL "iOS"))
    target_sources(WhatSon PRIVATE
            ${CMAKE_SOURCE_DIR}/src/extension/trial/WhatSonTrialSecureStore.cpp
            ${CMAKE_SOURCE_DIR}/src/extension/trial/WhatSonTrialInstallStore.cpp
            ${CMAKE_SOURCE_DIR}/src/extension/trial/WhatSonTrialActivationPolicy.cpp
    )
    target_include_directories(WhatSon PRIVATE
            ${CMAKE_SOURCE_DIR}/src/extension/trial
    )
    target_compile_definitions(WhatSon PRIVATE
            WHATSON_IS_TRIAL_BUILD=1
    )
endif ()

add_subdirectory(models/agenda)
add_subdirectory(models/calendar)
add_subdirectory(models/callout)
add_subdirectory(models/sensor)
add_subdirectory(editor)
add_subdirectory(file)
add_subdirectory(permissions)
add_subdirectory(platform)
add_subdirectory(policy)
add_subdirectory(qml)
add_subdirectory(register)
add_subdirectory(runtime)
add_subdirectory(store)
add_subdirectory(viewmodel)

set(WHATSON_RESOURCES_DIR "${CMAKE_SOURCE_DIR}/resources")
set(WHATSON_APP_ICON_DIR "${WHATSON_RESOURCES_DIR}/icons/app")
set(WHATSON_APP_ICON_DESKTOP_DIR "${WHATSON_APP_ICON_DIR}/desktop")
set(WHATSON_APP_ICON_IOS_DIR "${WHATSON_APP_ICON_DIR}/ios")
set(WHATSON_APP_ICON_ANDROID_DIR "${WHATSON_APP_ICON_DIR}/android")
set(WHATSON_APP_ICON_STORE_DIR "${WHATSON_APP_ICON_DIR}/store")
set(WHATSON_ILLUSTRATIONS_DIR "${WHATSON_RESOURCES_DIR}/illustrations")
set(WHATSON_ONBOARDING_ILLUSTRATION_PNG "${WHATSON_ILLUSTRATIONS_DIR}/onboarding/Onboarding.png")

set(WHATSON_APP_ICON_PNG "${WHATSON_APP_ICON_DESKTOP_DIR}/AppIcon.png")
set(WHATSON_APP_ICON_ICNS "${WHATSON_APP_ICON_DESKTOP_DIR}/AppIcon.icns")
set(WHATSON_APP_ICON_ICO "${WHATSON_APP_ICON_DESKTOP_DIR}/AppIcon.ico")
set(WHATSON_IOS_APP_ICON_FILES
        "${WHATSON_APP_ICON_IOS_DIR}/AppIcon-Notification@2x.png"
        "${WHATSON_APP_ICON_IOS_DIR}/AppIcon-Notification@3x.png"
        "${WHATSON_APP_ICON_IOS_DIR}/AppIcon-Settings@2x.png"
        "${WHATSON_APP_ICON_IOS_DIR}/AppIcon-Settings@3x.png"
        "${WHATSON_APP_ICON_IOS_DIR}/AppIcon-Spotlight-40@2x.png"
        "${WHATSON_APP_ICON_IOS_DIR}/AppIcon-Spotlight-40@3x.png"
        "${WHATSON_APP_ICON_IOS_DIR}/AppIcon-60@2x.png"
        "${WHATSON_APP_ICON_IOS_DIR}/AppIcon-60@3x.png"
        "${WHATSON_APP_ICON_IOS_DIR}/AppIcon-Notification~ipad.png"
        "${WHATSON_APP_ICON_IOS_DIR}/AppIcon-Notification@2x~ipad.png"
        "${WHATSON_APP_ICON_IOS_DIR}/AppIcon-Settings~ipad.png"
        "${WHATSON_APP_ICON_IOS_DIR}/AppIcon-Settings@2x~ipad.png"
        "${WHATSON_APP_ICON_IOS_DIR}/AppIcon-Spotlight-40~ipad.png"
        "${WHATSON_APP_ICON_IOS_DIR}/AppIcon-Spotlight-40@2x~ipad.png"
        "${WHATSON_APP_ICON_IOS_DIR}/AppIcon-76~ipad.png"
        "${WHATSON_APP_ICON_IOS_DIR}/AppIcon-76@2x~ipad.png"
        "${WHATSON_APP_ICON_IOS_DIR}/AppIcon-83.5@2x~ipad.png"
        "${WHATSON_APP_ICON_IOS_DIR}/AppIcon@ios~marketing.png"
)
set(WHATSON_IOS_BUNDLE_ICON_FILES "${WHATSON_APP_ICON_PNG}" ${WHATSON_IOS_APP_ICON_FILES})
list(REMOVE_DUPLICATES WHATSON_IOS_BUNDLE_ICON_FILES)

if (NOT DEFINED WHATSON_PLATFORM_DIR)
    set(WHATSON_PLATFORM_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../platform")
endif ()
if (NOT DEFINED WHATSON_APPLE_DIR)
    set(WHATSON_APPLE_DIR "${WHATSON_PLATFORM_DIR}/Apple")
endif ()
if (NOT DEFINED WHATSON_ANDROID_DIR)
    set(WHATSON_ANDROID_DIR "${WHATSON_PLATFORM_DIR}/Android")
endif ()
if (NOT DEFINED WHATSON_IOS_INFO_PLIST)
    set(WHATSON_IOS_INFO_PLIST "${WHATSON_APPLE_DIR}/iOS/Info.plist")
endif ()
if (NOT DEFINED WHATSON_DEFAULT_INFO_PLIST)
    set(WHATSON_DEFAULT_INFO_PLIST "${WHATSON_APPLE_DIR}/Info.plist")
endif ()
if (NOT DEFINED WHATSON_ENTITLEMENTS)
    set(WHATSON_ENTITLEMENTS "${WHATSON_APPLE_DIR}/WhatSon.entitlements")
endif ()
if (NOT DEFINED WHATSON_APPLE_BUNDLE_ID)
    set(WHATSON_APPLE_BUNDLE_ID "com.iisacc.app.whatson")
endif ()
if (NOT DEFINED WHATSON_ANDROID_PACKAGE_ID)
    set(WHATSON_ANDROID_PACKAGE_ID "com.iisacc.app.whatson")
endif ()

set(_whatson_android_package_source_dir "${WHATSON_ANDROID_DIR}")
add_subdirectory(cmake/resources)
add_subdirectory(cmake/defaults)

get_property(WHATSON_APP_QML_FILES GLOBAL PROPERTY WHATSON_APP_QML_FILES)
get_property(WHATSON_APP_QML_RESOURCE_FILES GLOBAL PROPERTY WHATSON_APP_QML_RESOURCE_FILES)

list(REMOVE_DUPLICATES WHATSON_APP_QML_FILES)
list(REMOVE_DUPLICATES WHATSON_APP_QML_RESOURCE_FILES)
list(SORT WHATSON_APP_QML_FILES)
list(SORT WHATSON_APP_QML_RESOURCE_FILES)

if (NOT WHATSON_APP_QML_FILES)
    message(FATAL_ERROR "No QML files were discovered under src/app/qml.")
endif ()

qt_add_qml_module(WhatSon
        URI WhatSon.App
        VERSION 1.0
        RESOURCE_PREFIX "/qt/qml"
        QML_FILES
        ${WHATSON_APP_QML_FILES}
        RESOURCES
        ${WHATSON_APP_QML_RESOURCE_FILES}
)

add_subdirectory(cmake/runtime)
