find_package(Qt6 6.5 REQUIRED COMPONENTS Quick QuickControls2 QuickDialogs2 Pdf PdfQuick)
find_package(LVRS CONFIG REQUIRED)
qt_policy(SET QTP0004 NEW)

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

if (EXISTS "${WHATSON_ONBOARDING_ILLUSTRATION_PNG}")
    set_source_files_properties("${WHATSON_ONBOARDING_ILLUSTRATION_PNG}" PROPERTIES
            QT_RESOURCE_ALIAS "Onboarding.png")
    qt_add_resources(WhatSon "WhatSonOnboardingIllustration"
            PREFIX
            "/whatson/illustrations/onboarding"
            FILES
            "${WHATSON_ONBOARDING_ILLUSTRATION_PNG}"
    )
endif ()

if (APPLE AND NOT CMAKE_SYSTEM_NAME STREQUAL "iOS" AND EXISTS "${WHATSON_APP_ICON_ICNS}")
    set_source_files_properties("${WHATSON_APP_ICON_ICNS}" PROPERTIES
            MACOSX_PACKAGE_LOCATION "Resources")
    target_sources(WhatSon PRIVATE "${WHATSON_APP_ICON_ICNS}")
endif ()

if (APPLE AND CMAKE_SYSTEM_NAME STREQUAL "iOS")
    set(_whatson_use_ios_asset_catalog FALSE)
    if (CMAKE_GENERATOR MATCHES "^Xcode$")
        set(_whatson_has_complete_ios_icon_set TRUE)
        foreach (_whatson_ios_icon_file IN LISTS WHATSON_IOS_APP_ICON_FILES)
            if (NOT EXISTS "${_whatson_ios_icon_file}")
                message(STATUS "WhatSon iOS icon asset missing, falling back to bundled PNG icons: ${_whatson_ios_icon_file}")
                set(_whatson_has_complete_ios_icon_set FALSE)
                break()
            endif ()
        endforeach ()

        if (_whatson_has_complete_ios_icon_set)
            set(_whatson_ios_asset_catalog_dir "${CMAKE_CURRENT_BINARY_DIR}/WhatSonIcons.xcassets")
            set(_whatson_ios_app_iconset_dir "${_whatson_ios_asset_catalog_dir}/AppIcon.appiconset")
            file(REMOVE_RECURSE "${_whatson_ios_asset_catalog_dir}")
            file(MAKE_DIRECTORY "${_whatson_ios_app_iconset_dir}")

            file(WRITE "${_whatson_ios_asset_catalog_dir}/Contents.json" [[
{
  "info" : {
    "author" : "xcode",
    "version" : 1
  }
}
]])

            foreach (_whatson_ios_icon_file IN LISTS WHATSON_IOS_APP_ICON_FILES)
                get_filename_component(_whatson_ios_icon_name "${_whatson_ios_icon_file}" NAME)
                file(COPY_FILE "${_whatson_ios_icon_file}"
                        "${_whatson_ios_app_iconset_dir}/${_whatson_ios_icon_name}"
                        ONLY_IF_DIFFERENT)
            endforeach ()

            file(WRITE "${_whatson_ios_app_iconset_dir}/Contents.json" [[
{
  "images" : [
    { "filename" : "AppIcon-Notification@2x.png", "idiom" : "iphone", "scale" : "2x", "size" : "20x20" },
    { "filename" : "AppIcon-Notification@3x.png", "idiom" : "iphone", "scale" : "3x", "size" : "20x20" },
    { "filename" : "AppIcon-Settings@2x.png", "idiom" : "iphone", "scale" : "2x", "size" : "29x29" },
    { "filename" : "AppIcon-Settings@3x.png", "idiom" : "iphone", "scale" : "3x", "size" : "29x29" },
    { "filename" : "AppIcon-Spotlight-40@2x.png", "idiom" : "iphone", "scale" : "2x", "size" : "40x40" },
    { "filename" : "AppIcon-Spotlight-40@3x.png", "idiom" : "iphone", "scale" : "3x", "size" : "40x40" },
    { "filename" : "AppIcon-60@2x.png", "idiom" : "iphone", "scale" : "2x", "size" : "60x60" },
    { "filename" : "AppIcon-60@3x.png", "idiom" : "iphone", "scale" : "3x", "size" : "60x60" },
    { "filename" : "AppIcon-Notification~ipad.png", "idiom" : "ipad", "scale" : "1x", "size" : "20x20" },
    { "filename" : "AppIcon-Notification@2x~ipad.png", "idiom" : "ipad", "scale" : "2x", "size" : "20x20" },
    { "filename" : "AppIcon-Settings~ipad.png", "idiom" : "ipad", "scale" : "1x", "size" : "29x29" },
    { "filename" : "AppIcon-Settings@2x~ipad.png", "idiom" : "ipad", "scale" : "2x", "size" : "29x29" },
    { "filename" : "AppIcon-Spotlight-40~ipad.png", "idiom" : "ipad", "scale" : "1x", "size" : "40x40" },
    { "filename" : "AppIcon-Spotlight-40@2x~ipad.png", "idiom" : "ipad", "scale" : "2x", "size" : "40x40" },
    { "filename" : "AppIcon-76~ipad.png", "idiom" : "ipad", "scale" : "1x", "size" : "76x76" },
    { "filename" : "AppIcon-76@2x~ipad.png", "idiom" : "ipad", "scale" : "2x", "size" : "76x76" },
    { "filename" : "AppIcon-83.5@2x~ipad.png", "idiom" : "ipad", "scale" : "2x", "size" : "83.5x83.5" },
    { "filename" : "AppIcon@ios~marketing.png", "idiom" : "ios-marketing", "scale" : "1x", "size" : "1024x1024" }
  ],
  "info" : {
    "author" : "xcode",
    "version" : 1
  }
}
]])

            set_source_files_properties("${_whatson_ios_asset_catalog_dir}" PROPERTIES
                    MACOSX_PACKAGE_LOCATION "Resources"
                    XCODE_EXPLICIT_FILE_TYPE "folder.assetcatalog")
            target_sources(WhatSon PRIVATE "${_whatson_ios_asset_catalog_dir}")
            set_target_properties(WhatSon PROPERTIES
                    XCODE_ATTRIBUTE_ASSETCATALOG_COMPILER_APPICON_NAME "AppIcon")
            set(_whatson_use_ios_asset_catalog TRUE)
        endif ()
    endif ()

    if (NOT _whatson_use_ios_asset_catalog)
        foreach (_whatson_ios_bundle_icon_file IN LISTS WHATSON_IOS_BUNDLE_ICON_FILES)
            if (NOT EXISTS "${_whatson_ios_bundle_icon_file}")
                continue()
            endif ()
            set_source_files_properties("${_whatson_ios_bundle_icon_file}" PROPERTIES
                    MACOSX_PACKAGE_LOCATION ".")
            target_sources(WhatSon PRIVATE "${_whatson_ios_bundle_icon_file}")
        endforeach ()
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

set(_whatson_android_package_source_dir "${WHATSON_ANDROID_DIR}")
if (ANDROID AND EXISTS "${WHATSON_ANDROID_DIR}")
    set(_whatson_android_package_source_dir "${CMAKE_CURRENT_BINARY_DIR}/android-package")
    file(REMOVE_RECURSE "${_whatson_android_package_source_dir}")
    file(MAKE_DIRECTORY "${_whatson_android_package_source_dir}")
    file(COPY "${WHATSON_ANDROID_DIR}/" DESTINATION "${_whatson_android_package_source_dir}")

    set(_whatson_android_default_icon_dst_dir "${_whatson_android_package_source_dir}/res/drawable")
    foreach (_whatson_android_default_icon_src IN ITEMS
            "${WHATSON_APP_ICON_ANDROID_DIR}/mdpi/AppIcon.png"
            "${WHATSON_APP_ICON_ANDROID_DIR}/xhdpi/AppIcon.png"
            "${WHATSON_APP_ICON_PNG}")
        if (NOT EXISTS "${_whatson_android_default_icon_src}")
            continue()
        endif ()

        file(MAKE_DIRECTORY "${_whatson_android_default_icon_dst_dir}")
        file(COPY_FILE "${_whatson_android_default_icon_src}"
                "${_whatson_android_default_icon_dst_dir}/app_icon.png"
                ONLY_IF_DIFFERENT)
        break()
    endforeach ()

    foreach (_whatson_android_density IN ITEMS ldpi mdpi hdpi xhdpi xxhdpi xxxhdpi)
        set(_whatson_android_icon_src "${WHATSON_APP_ICON_ANDROID_DIR}/${_whatson_android_density}/AppIcon.png")
        if (NOT EXISTS "${_whatson_android_icon_src}")
            continue()
        endif ()

        set(_whatson_android_icon_dst_dir
                "${_whatson_android_package_source_dir}/res/drawable-${_whatson_android_density}")
        file(MAKE_DIRECTORY "${_whatson_android_icon_dst_dir}")
        file(COPY_FILE "${_whatson_android_icon_src}"
                "${_whatson_android_icon_dst_dir}/app_icon.png"
                ONLY_IF_DIFFERENT)
    endforeach ()

    set(_whatson_android_googleplay_icon_src "${WHATSON_APP_ICON_STORE_DIR}/googleplay/AppIcon.png")
    if (EXISTS "${_whatson_android_googleplay_icon_src}")
        set(_whatson_android_googleplay_icon_dir "${_whatson_android_package_source_dir}/googleplay")
        file(MAKE_DIRECTORY "${_whatson_android_googleplay_icon_dir}")
        file(COPY_FILE "${_whatson_android_googleplay_icon_src}"
                "${_whatson_android_googleplay_icon_dir}/AppIcon.png"
                ONLY_IF_DIFFERENT)
    endif ()
endif ()

if (WIN32 AND EXISTS "${WHATSON_APP_ICON_ICO}")
    file(TO_NATIVE_PATH "${WHATSON_APP_ICON_ICO}" _whatson_windows_icon_native_path)
    string(REPLACE "\\" "\\\\" _whatson_windows_icon_native_path "${_whatson_windows_icon_native_path}")
    set(_whatson_windows_app_icon_rc "${CMAKE_CURRENT_BINARY_DIR}/WhatSonAppIcon.rc")
    file(WRITE "${_whatson_windows_app_icon_rc}"
            "IDI_APP_ICON ICON \"${_whatson_windows_icon_native_path}\"\n")
    target_sources(WhatSon PRIVATE "${_whatson_windows_app_icon_rc}")
endif ()

set(_whatson_lvrs_defaults_args
        TARGET WhatSon
        APPLE_BUNDLE_ID "${WHATSON_APPLE_BUNDLE_ID}"
        ANDROID_PACKAGE_ID "${WHATSON_ANDROID_PACKAGE_ID}"
        ANDROID_PACKAGE_SOURCE_DIR "${_whatson_android_package_source_dir}"
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
endif ()

target_link_libraries(WhatSon
        PRIVATE
        Qt6::Pdf
        Qt6::PdfQuick
        Qt6::Quick
        Qt6::QuickDialogs2
        Qt6::QuickControls2
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
