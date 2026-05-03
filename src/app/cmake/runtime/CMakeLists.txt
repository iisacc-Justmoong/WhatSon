if (APPLE AND CMAKE_SYSTEM_NAME STREQUAL "iOS")
    # The clean iOS Xcode export tree does not have Qt-generated .qrc files yet during
    # target finalization, so configure-time qmlimportscanner fails before project generation.
    set_target_properties(WhatSon PROPERTIES
            QT_QML_MODULE_NO_IMPORT_SCAN TRUE)

    # With import scanning disabled, iOS must link the full static QML plugin closure that
    # qmlimportscanner would normally resolve, including transitive plugins hidden behind
    # QT_IS_PLUGIN_GENEX in the imported Qt plugin targets.
    set(_whatson_ios_qml_plugins
            Qt6::qtquick2plugin
            Qt6::quickwindow
            Qt6::qmlplugin
            Qt6::modelsplugin
            Qt6::workerscriptplugin
            Qt6::qquicklayoutsplugin
            Qt6::qtquicktemplates2plugin
            Qt6::qtquickcontrols2implplugin
            Qt6::qtquickcontrols2basicstyleimplplugin
            Qt6::qtquickcontrols2iosstyleimplplugin
            Qt6::qtquickcontrols2plugin
            Qt6::qtquickcontrols2basicstyleplugin
            Qt6::qtquickcontrols2iosstyleplugin
            Qt6::qtquickdialogs2quickimplplugin
            Qt6::qtquickdialogsplugin
            Qt6::qmlshapesplugin
            Qt6::PdfQuickplugin)
    foreach (_whatson_ios_qml_plugin IN LISTS _whatson_ios_qml_plugins)
        if (TARGET "${_whatson_ios_qml_plugin}")
            target_link_libraries(WhatSon PRIVATE "${_whatson_ios_qml_plugin}")
        endif ()
    endforeach ()

    set(_whatson_disable_qt_permission_plugins FALSE)
    if (WHATSON_IOS_QT_PERMISSION_PLUGIN_POLICY STREQUAL "exclude")
        set(_whatson_disable_qt_permission_plugins TRUE)
    elseif (WHATSON_IOS_QT_PERMISSION_PLUGIN_POLICY STREQUAL "auto")
        if (CMAKE_OSX_SYSROOT MATCHES "iphonesimulator")
            # Some Qt iOS kits ship device-only arm64 slices for permissions plugins, which
            # breaks Apple Silicon simulator links unless the plugin type is excluded.
            set(_whatson_disable_qt_permission_plugins TRUE)
        endif ()
    elseif (NOT WHATSON_IOS_QT_PERMISSION_PLUGIN_POLICY STREQUAL "include")
        message(FATAL_ERROR
                "Unsupported WHATSON_IOS_QT_PERMISSION_PLUGIN_POLICY value: "
                "${WHATSON_IOS_QT_PERMISSION_PLUGIN_POLICY}. Expected auto, include, or exclude.")
    endif ()

    if (_whatson_disable_qt_permission_plugins)
        qt_import_plugins(WhatSon EXCLUDE_BY_TYPE permissions)
        target_compile_definitions(WhatSon PRIVATE WHATSON_DISABLE_QT_PERMISSION_REQUESTS=1)
    endif ()
endif ()

target_link_libraries(WhatSon
        PRIVATE
        Qt6::Pdf
        Qt6::PdfQuick
        Qt6::Quick
        Qt6::QuickDialogs2
        Qt6::QuickControls2
        iiXml::iiXml
        iiHtmlBlock::iiHtmlBlock
)

if (UNIX AND NOT APPLE)
    set_target_properties(WhatSon PROPERTIES
            INSTALL_RPATH "$ORIGIN/../lib")
endif ()

set(_whatson_app_runtime_output_dir "$<1:${CMAKE_CURRENT_BINARY_DIR}/bin>")
if (NOT ANDROID AND NOT (APPLE AND CMAKE_SYSTEM_NAME STREQUAL "iOS"))
    set(_whatson_app_runtime_output_dir "$<1:${CMAKE_BINARY_DIR}>")
endif ()
set_target_properties(WhatSon PROPERTIES
        OUTPUT_NAME "WhatSon"
        RUNTIME_OUTPUT_DIRECTORY "${_whatson_app_runtime_output_dir}"
)

if (WIN32)
    set_target_properties(WhatSon PROPERTIES
            WIN32_EXECUTABLE ON)
endif ()

if (APPLE)
    set_target_properties(WhatSon PROPERTIES
            MACOSX_BUNDLE ON
            MACOSX_BUNDLE_BUNDLE_NAME "WhatSon"
    )

    if (NOT CMAKE_SYSTEM_NAME STREQUAL "iOS" AND EXISTS "${WHATSON_APP_ICON_ICNS}")
        set_target_properties(WhatSon PROPERTIES
                MACOSX_BUNDLE_ICON_FILE "AppIcon.icns")
    endif ()
endif ()

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

    if (WHATSON_IS_TRIAL_BUILD AND NOT CMAKE_SYSTEM_NAME STREQUAL "iOS")
        find_library(WHATSON_SECURITY_FRAMEWORK Security REQUIRED)
        target_link_libraries(WhatSon PRIVATE "${WHATSON_SECURITY_FRAMEWORK}")
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
