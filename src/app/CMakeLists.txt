find_package(Qt6 6.5 REQUIRED COMPONENTS Quick QuickControls2)
find_package(LVRS CONFIG REQUIRED)
qt_policy(SET QTP0004 NEW)

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

if (APPLE)
    list(FILTER WHATSON_APP_SOURCE_FILES EXCLUDE REGEX "ApplePermissionBridge_stub\\.cpp$")
else ()
    list(FILTER WHATSON_APP_SOURCE_FILES EXCLUDE REGEX "\\.(m|mm)$")
endif ()

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

set(WHATSON_APP_ICON_PNG "${CMAKE_SOURCE_DIR}/resources/AppIcon.png")

if (EXISTS "${WHATSON_APP_ICON_PNG}")
    set_source_files_properties("${WHATSON_APP_ICON_PNG}" PROPERTIES
            QT_RESOURCE_ALIAS "AppIcon.png")
    qt_add_resources(WhatSon "WhatSonAppIcon"
            PREFIX
            "/whatson"
            FILES
            "${WHATSON_APP_ICON_PNG}"
    )
endif ()

if (APPLE AND CMAKE_SYSTEM_NAME STREQUAL "iOS")
    if (EXISTS "${WHATSON_APP_ICON_PNG}")
        set_source_files_properties("${WHATSON_APP_ICON_PNG}" PROPERTIES
                MACOSX_PACKAGE_LOCATION ".")
        target_sources(WhatSon PRIVATE "${WHATSON_APP_ICON_PNG}")
    endif ()
endif ()

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

if (APPLE AND CMAKE_SYSTEM_NAME STREQUAL "iOS")
    # The clean iOS Xcode export tree does not have Qt-generated .qrc files yet during
    # target finalization, so configure-time qmlimportscanner fails before project generation.
    set_target_properties(WhatSon PROPERTIES
            QT_QML_MODULE_NO_IMPORT_SCAN TRUE)
endif ()

target_link_libraries(WhatSon
        PRIVATE
        Qt6::Quick
        Qt6::QuickControls2
)

if (APPLE)
    find_library(WHATSON_EVENTKIT_FRAMEWORK EventKit)
    if (WHATSON_EVENTKIT_FRAMEWORK)
        target_link_libraries(WhatSon PRIVATE "${WHATSON_EVENTKIT_FRAMEWORK}")
    endif ()

    find_library(WHATSON_PHOTOS_FRAMEWORK Photos)
    if (WHATSON_PHOTOS_FRAMEWORK)
        target_link_libraries(WhatSon PRIVATE "${WHATSON_PHOTOS_FRAMEWORK}")
    endif ()

    if (NOT CMAKE_SYSTEM_NAME STREQUAL "iOS")
        find_library(WHATSON_APPLICATION_SERVICES_FRAMEWORK ApplicationServices)
        if (WHATSON_APPLICATION_SERVICES_FRAMEWORK)
            target_link_libraries(WhatSon PRIVATE "${WHATSON_APPLICATION_SERVICES_FRAMEWORK}")
        endif ()
    endif ()
endif ()

if (APPLE AND CMAKE_SYSTEM_NAME STREQUAL "iOS")
    set(_whatson_lvrs_qml_root "")
    if (DEFINED LVRS_QML_MODULE_PATH AND EXISTS "${LVRS_QML_MODULE_PATH}/qmldir")
        set(_whatson_lvrs_qml_root "${LVRS_QML_MODULE_PATH}")
    elseif (DEFINED LVRS_ACTIVE_PREFIX AND EXISTS "${LVRS_ACTIVE_PREFIX}/lib/qt6/qml/LVRS/qmldir")
        set(_whatson_lvrs_qml_root "${LVRS_ACTIVE_PREFIX}/lib/qt6/qml/LVRS")
    endif ()

    if (NOT _whatson_lvrs_qml_root STREQUAL "")
        file(GLOB _whatson_lvrs_plugin_candidates
                "${_whatson_lvrs_qml_root}/LVRSCoreplugin${CMAKE_STATIC_LIBRARY_SUFFIX}"
                "${_whatson_lvrs_qml_root}/${CMAKE_STATIC_LIBRARY_PREFIX}LVRSCoreplugin${CMAKE_STATIC_LIBRARY_SUFFIX}"
                "${_whatson_lvrs_qml_root}/Release/LVRSCoreplugin${CMAKE_STATIC_LIBRARY_SUFFIX}"
                "${_whatson_lvrs_qml_root}/Release/${CMAKE_STATIC_LIBRARY_PREFIX}LVRSCoreplugin${CMAKE_STATIC_LIBRARY_SUFFIX}"
                "${_whatson_lvrs_qml_root}/LVRSCoreplugin${CMAKE_SHARED_LIBRARY_SUFFIX}"
                "${_whatson_lvrs_qml_root}/${CMAKE_SHARED_LIBRARY_PREFIX}LVRSCoreplugin${CMAKE_SHARED_LIBRARY_SUFFIX}"
                "${_whatson_lvrs_qml_root}/Release/LVRSCoreplugin${CMAKE_SHARED_LIBRARY_SUFFIX}"
                "${_whatson_lvrs_qml_root}/Release/${CMAKE_SHARED_LIBRARY_PREFIX}LVRSCoreplugin${CMAKE_SHARED_LIBRARY_SUFFIX}"
        )

        if (NOT _whatson_lvrs_plugin_candidates)
            set(_whatson_lvrs_qmldir_override "")
            if (EXISTS "${_whatson_lvrs_qml_root}/qmldir")
                file(STRINGS "${_whatson_lvrs_qml_root}/qmldir" _whatson_lvrs_qmldir_lines)
                set(_whatson_lvrs_qmldir_sanitized "")
                foreach (_whatson_lvrs_qmldir_line IN LISTS _whatson_lvrs_qmldir_lines)
                    if (_whatson_lvrs_qmldir_line MATCHES "^(linktarget|classname|plugin|optional plugin)[ ]")
                        continue()
                    endif ()
                    string(APPEND _whatson_lvrs_qmldir_sanitized "${_whatson_lvrs_qmldir_line}\n")
                endforeach ()
                set(_whatson_lvrs_qmldir_override "${CMAKE_CURRENT_BINARY_DIR}/WhatSon_lvrs_fallback_qmldir")
                file(WRITE "${_whatson_lvrs_qmldir_override}" "${_whatson_lvrs_qmldir_sanitized}")
                set_source_files_properties("${_whatson_lvrs_qmldir_override}" PROPERTIES QT_RESOURCE_ALIAS "qmldir")
            endif ()

            file(GLOB_RECURSE _whatson_lvrs_qml_entries
                    CONFIGURE_DEPENDS
                    RELATIVE "${_whatson_lvrs_qml_root}"
                    "${_whatson_lvrs_qml_root}/*"
            )

            set(_whatson_lvrs_qml_files)
            foreach (_whatson_lvrs_entry IN LISTS _whatson_lvrs_qml_entries)
                if (IS_DIRECTORY "${_whatson_lvrs_qml_root}/${_whatson_lvrs_entry}")
                    continue()
                endif ()
                if (_whatson_lvrs_entry MATCHES "(^|/)\\.DS_Store$")
                    continue()
                endif ()
                if (_whatson_lvrs_entry MATCHES "LVRSCore_qml_module_dir_map\\.qrc$")
                    continue()
                endif ()
                if (_whatson_lvrs_entry STREQUAL "qmldir")
                    if (NOT _whatson_lvrs_qmldir_override STREQUAL "")
                        list(APPEND _whatson_lvrs_qml_files "${_whatson_lvrs_qmldir_override}")
                    else ()
                        list(APPEND _whatson_lvrs_qml_files "${_whatson_lvrs_qml_root}/${_whatson_lvrs_entry}")
                    endif ()
                    continue()
                endif ()
                list(APPEND _whatson_lvrs_qml_files "${_whatson_lvrs_qml_root}/${_whatson_lvrs_entry}")
            endforeach ()

            if (_whatson_lvrs_qml_files)
                list(SORT _whatson_lvrs_qml_files)
                message(STATUS "WhatSon iOS fallback: embedding LVRS QML module resources from ${_whatson_lvrs_qml_root}")
                qt_add_resources(WhatSon "WhatSonLvrsIosFallbackModule"
                        PREFIX
                        "/qt/qml/LVRS"
                        BASE
                        "${_whatson_lvrs_qml_root}"
                        FILES
                        ${_whatson_lvrs_qml_files}
                )
            endif ()
        endif ()
    endif ()
endif ()

if (APPLE AND NOT CMAKE_SYSTEM_NAME STREQUAL "iOS")
    # Host build: create a filesystem LVRS module overlay without "prefer :/qt/qml/LVRS/".
    # This prevents loading stale qrc-side LVRS QML and forces the installed module files.
    set(_whatson_lvrs_host_qmldir_path "")
    if (DEFINED LVRS_QML_MODULE_PATH AND EXISTS "${LVRS_QML_MODULE_PATH}/qmldir")
        set(_whatson_lvrs_host_qmldir_path "${LVRS_QML_MODULE_PATH}/qmldir")
    elseif (DEFINED LVRS_ACTIVE_PREFIX AND EXISTS "${LVRS_ACTIVE_PREFIX}/lib/qt6/qml/LVRS/qmldir")
        set(_whatson_lvrs_host_qmldir_path "${LVRS_ACTIVE_PREFIX}/lib/qt6/qml/LVRS/qmldir")
    endif ()

    if (NOT _whatson_lvrs_host_qmldir_path STREQUAL "")
        get_filename_component(_whatson_lvrs_host_module_root "${_whatson_lvrs_host_qmldir_path}" DIRECTORY)

        file(STRINGS "${_whatson_lvrs_host_qmldir_path}" _whatson_lvrs_host_qmldir_lines)
        set(_whatson_lvrs_host_qmldir_runtime "")
        foreach (_whatson_lvrs_host_qmldir_line IN LISTS _whatson_lvrs_host_qmldir_lines)
            if (_whatson_lvrs_host_qmldir_line MATCHES "^prefer[ ]")
                continue()
            endif ()
            string(APPEND _whatson_lvrs_host_qmldir_runtime "${_whatson_lvrs_host_qmldir_line}\n")
        endforeach ()

        set(_whatson_lvrs_host_import_root "${CMAKE_CURRENT_BINARY_DIR}/lvrs_runtime_qml")
        set(_whatson_lvrs_host_runtime_module_dir "${_whatson_lvrs_host_import_root}/LVRS")
        file(MAKE_DIRECTORY "${_whatson_lvrs_host_runtime_module_dir}")
        file(WRITE "${_whatson_lvrs_host_runtime_module_dir}/qmldir" "${_whatson_lvrs_host_qmldir_runtime}")

        set(_whatson_lvrs_host_qml_src "${_whatson_lvrs_host_module_root}/qml")
        set(_whatson_lvrs_host_qml_dst "${_whatson_lvrs_host_runtime_module_dir}/qml")
        if (EXISTS "${_whatson_lvrs_host_qml_dst}" OR IS_SYMLINK "${_whatson_lvrs_host_qml_dst}")
            file(REMOVE_RECURSE "${_whatson_lvrs_host_qml_dst}")
        endif ()
        file(COPY "${_whatson_lvrs_host_qml_src}" DESTINATION "${_whatson_lvrs_host_runtime_module_dir}")

        set(_whatson_lvrs_host_abstract_input_bar
                "${_whatson_lvrs_host_qml_dst}/components/control/input/AbstractInputBar.qml")
        if (EXISTS "${_whatson_lvrs_host_abstract_input_bar}")
            file(READ "${_whatson_lvrs_host_abstract_input_bar}" _whatson_lvrs_host_abstract_input_bar_source)
            string(REPLACE
                    "    property alias selectByKeyboard: inputField.selectByKeyboard\n"
                    ""
                    _whatson_lvrs_host_abstract_input_bar_patched
                    "${_whatson_lvrs_host_abstract_input_bar_source}")
            string(REPLACE
                    "    property alias selectByKeyboard: inputField.selectByKeyboard\r\n"
                    ""
                    _whatson_lvrs_host_abstract_input_bar_patched
                    "${_whatson_lvrs_host_abstract_input_bar_patched}")
            string(REPLACE
                    "        selectByKeyboard: true\n"
                    ""
                    _whatson_lvrs_host_abstract_input_bar_patched
                    "${_whatson_lvrs_host_abstract_input_bar_patched}")
            string(REPLACE
                    "        selectByKeyboard: true\r\n"
                    ""
                    _whatson_lvrs_host_abstract_input_bar_patched
                    "${_whatson_lvrs_host_abstract_input_bar_patched}")
            file(WRITE "${_whatson_lvrs_host_abstract_input_bar}"
                    "${_whatson_lvrs_host_abstract_input_bar_patched}")
        endif ()

        file(GLOB_RECURSE _whatson_lvrs_host_qml_patch_files
                CONFIGURE_DEPENDS
                "${_whatson_lvrs_host_qml_dst}/*.qml")
        foreach (_whatson_lvrs_host_qml_patch_file IN LISTS _whatson_lvrs_host_qml_patch_files)
            file(READ "${_whatson_lvrs_host_qml_patch_file}" _whatson_lvrs_host_qml_patch_source)
            string(REPLACE
                    "\"projectStructure\""
                    "\"generalprojectStructure\""
                    _whatson_lvrs_host_qml_patch_result
                    "${_whatson_lvrs_host_qml_patch_source}")
            file(WRITE "${_whatson_lvrs_host_qml_patch_file}" "${_whatson_lvrs_host_qml_patch_result}")
        endforeach ()

        foreach (_whatson_lvrs_host_entry IN ITEMS "resources" "LVRSCore.qmltypes")
            set(_whatson_lvrs_host_entry_src "${_whatson_lvrs_host_module_root}/${_whatson_lvrs_host_entry}")
            set(_whatson_lvrs_host_entry_dst "${_whatson_lvrs_host_runtime_module_dir}/${_whatson_lvrs_host_entry}")
            if (NOT EXISTS "${_whatson_lvrs_host_entry_src}")
                continue()
            endif ()
            if (EXISTS "${_whatson_lvrs_host_entry_dst}" OR IS_SYMLINK "${_whatson_lvrs_host_entry_dst}")
                file(REMOVE_RECURSE "${_whatson_lvrs_host_entry_dst}")
            endif ()
            file(CREATE_LINK "${_whatson_lvrs_host_entry_src}" "${_whatson_lvrs_host_entry_dst}" SYMBOLIC)
        endforeach ()

        foreach (_whatson_lvrs_host_plugin_src IN ITEMS
                "${_whatson_lvrs_host_module_root}/libLVRSCoreplugin${CMAKE_SHARED_LIBRARY_SUFFIX}"
                "${_whatson_lvrs_host_module_root}/LVRSCoreplugin${CMAKE_SHARED_LIBRARY_SUFFIX}")
            if (NOT EXISTS "${_whatson_lvrs_host_plugin_src}")
                continue()
            endif ()
            get_filename_component(_whatson_lvrs_host_plugin_name "${_whatson_lvrs_host_plugin_src}" NAME)
            set(_whatson_lvrs_host_plugin_dst
                    "${_whatson_lvrs_host_runtime_module_dir}/${_whatson_lvrs_host_plugin_name}")
            if (EXISTS "${_whatson_lvrs_host_plugin_dst}" OR IS_SYMLINK "${_whatson_lvrs_host_plugin_dst}")
                file(REMOVE "${_whatson_lvrs_host_plugin_dst}")
            endif ()
            file(CREATE_LINK "${_whatson_lvrs_host_plugin_src}" "${_whatson_lvrs_host_plugin_dst}" SYMBOLIC)
            break()
        endforeach ()

        target_compile_definitions(WhatSon PRIVATE
                WHATSON_USE_LVRS_DYNAMIC_QML_IMPORT=1
                WHATSON_LVRS_RUNTIME_IMPORT_ROOT="${_whatson_lvrs_host_import_root}")
    endif ()
endif ()

lvrs_configure_qml_app(WhatSon)

set_target_properties(WhatSon PROPERTIES
        MACOSX_BUNDLE ON
        OUTPUT_NAME "WhatSon"
        MACOSX_BUNDLE_BUNDLE_NAME "WhatSon"
)
