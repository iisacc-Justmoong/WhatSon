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

if (APPLE AND EXISTS "${WHATSON_APP_ICON_ICNS}")
    set_source_files_properties("${WHATSON_APP_ICON_ICNS}" PROPERTIES
            MACOSX_PACKAGE_LOCATION "Resources")
    target_sources(WhatSon PRIVATE "${WHATSON_APP_ICON_ICNS}")
    set(WHATSON_MACOS_POST_BUILD_BUNDLE_ICON_FILE
            "${WHATSON_APP_ICON_ICNS}"
            PARENT_SCOPE)
endif ()

if (WIN32 AND EXISTS "${WHATSON_APP_ICON_ICO}")
    file(TO_NATIVE_PATH "${WHATSON_APP_ICON_ICO}" _whatson_windows_icon_native_path)
    string(REPLACE "\\" "\\\\" _whatson_windows_icon_native_path "${_whatson_windows_icon_native_path}")
    set(_whatson_windows_app_icon_rc "${CMAKE_CURRENT_BINARY_DIR}/WhatSonAppIcon.rc")
    file(WRITE "${_whatson_windows_app_icon_rc}"
            "IDI_APP_ICON ICON \"${_whatson_windows_icon_native_path}\"\n")
    target_sources(WhatSon PRIVATE "${_whatson_windows_app_icon_rc}")
endif ()
