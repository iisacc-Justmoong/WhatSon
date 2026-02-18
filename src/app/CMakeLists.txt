find_package(Qt6 6.5 REQUIRED COMPONENTS Quick QuickControls2)
find_package(LVRS CONFIG REQUIRED)
qt_policy(SET QTP0004 NEW)

file(GLOB_RECURSE WHATSON_APP_SOURCE_FILES
        CONFIGURE_DEPENDS
        RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
        "*.c"
        "*.cc"
        "*.cpp"
        "*.cxx"
        "*.h"
        "*.hh"
        "*.hpp"
        "*.hxx"
        "*.m"
        "*.mm"
)
list(SORT WHATSON_APP_SOURCE_FILES)

if (NOT WHATSON_APP_SOURCE_FILES)
    message(FATAL_ERROR "No source files were discovered under src/app.")
endif ()

file(GLOB_RECURSE WHATSON_APP_HEADER_FILES
        CONFIGURE_DEPENDS
        RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
        "*.h"
        "*.hh"
        "*.hpp"
        "*.hxx"
)
list(SORT WHATSON_APP_HEADER_FILES)

qt_add_executable(WhatSon ${WHATSON_APP_SOURCE_FILES})

set(WHATSON_APP_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}")
foreach (_header_file IN LISTS WHATSON_APP_HEADER_FILES)
    get_filename_component(_header_dir "${_header_file}" DIRECTORY)
    if (_header_dir STREQUAL "")
        continue()
    endif ()

    set(_include_dir_cursor "${_header_dir}")
    while (NOT _include_dir_cursor STREQUAL "")
        list(APPEND WHATSON_APP_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/${_include_dir_cursor}")
        get_filename_component(_include_dir_cursor "${_include_dir_cursor}" DIRECTORY)
        if (_include_dir_cursor STREQUAL ".")
            set(_include_dir_cursor "")
        endif ()
    endwhile ()
endforeach ()
list(REMOVE_DUPLICATES WHATSON_APP_INCLUDE_DIRS)
list(SORT WHATSON_APP_INCLUDE_DIRS)

target_include_directories(WhatSon PRIVATE ${WHATSON_APP_INCLUDE_DIRS})

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
    set(WHATSON_APPLE_BUNDLE_ID "com.lvrs.whatson")
endif ()
if (NOT DEFINED WHATSON_ANDROID_PACKAGE_ID)
    set(WHATSON_ANDROID_PACKAGE_ID "com.lvrs.whatson")
endif ()

set(_whatson_lvrs_defaults_args
        TARGET WhatSon
        APPLE_BUNDLE_ID "${WHATSON_APPLE_BUNDLE_ID}"
        ANDROID_PACKAGE_ID "${WHATSON_ANDROID_PACKAGE_ID}"
        ANDROID_PACKAGE_SOURCE_DIR "${WHATSON_ANDROID_DIR}"
)

set(_whatson_apple_info_plist "")
if (APPLE AND CMAKE_SYSTEM_NAME STREQUAL "iOS")
    if (EXISTS "${WHATSON_IOS_INFO_PLIST}")
        set(_whatson_apple_info_plist "${WHATSON_IOS_INFO_PLIST}")
    elseif (EXISTS "${WHATSON_DEFAULT_INFO_PLIST}")
        set(_whatson_apple_info_plist "${WHATSON_DEFAULT_INFO_PLIST}")
    endif ()
elseif (APPLE)
    if (EXISTS "${WHATSON_DEFAULT_INFO_PLIST}")
        set(_whatson_apple_info_plist "${WHATSON_DEFAULT_INFO_PLIST}")
    elseif (EXISTS "${WHATSON_IOS_INFO_PLIST}")
        set(_whatson_apple_info_plist "${WHATSON_IOS_INFO_PLIST}")
    endif ()
endif ()
if (NOT _whatson_apple_info_plist STREQUAL "")
    list(APPEND _whatson_lvrs_defaults_args APPLE_INFO_PLIST "${_whatson_apple_info_plist}")
endif ()
if (EXISTS "${WHATSON_ENTITLEMENTS}")
    list(APPEND _whatson_lvrs_defaults_args APPLE_ENTITLEMENTS "${WHATSON_ENTITLEMENTS}")
endif ()

lvrs_configure_project_defaults(${_whatson_lvrs_defaults_args})

file(GLOB_RECURSE WHATSON_APP_QML_FILES
        CONFIGURE_DEPENDS
        RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
        "qml/*.qml"
)
list(SORT WHATSON_APP_QML_FILES)

if (NOT WHATSON_APP_QML_FILES)
    message(FATAL_ERROR "No QML files were discovered under src/app/qml.")
endif ()

qt_add_qml_module(WhatSon
        URI WhatSon.App
        VERSION 1.0
        RESOURCE_PREFIX "/qt/qml"
        QML_FILES
        ${WHATSON_APP_QML_FILES}
)

target_link_libraries(WhatSon
        PRIVATE
        Qt6::Quick
        Qt6::QuickControls2
)

lvrs_configure_qml_app(WhatSon)

set_target_properties(WhatSon PROPERTIES
        MACOSX_BUNDLE ON
        OUTPUT_NAME "WhatSon"
        MACOSX_BUNDLE_BUNDLE_NAME "WhatSon"
)
