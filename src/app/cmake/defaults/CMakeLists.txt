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
