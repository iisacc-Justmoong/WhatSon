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

set(_whatson_android_package_source_dir "${_whatson_android_package_source_dir}" PARENT_SCOPE)
