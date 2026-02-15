find_package(Qt6 6.5 REQUIRED COMPONENTS Quick QuickControls2)
find_package(LVRS CONFIG REQUIRED)
qt_policy(SET QTP0004 NEW)

qt_add_executable(WhatSon
        main.cpp
        hub/WhatSonHubCreator.cpp
        hub/WhatSonHubCreator.hpp
        hub/WhatSonWorkspaceHubCreator.cpp
        hub/WhatSonWorkspaceHubCreator.hpp
        note/WhatSonNoteCreator.hpp
        note/WhatSonNoteBodyCreator.hpp
        note/WhatSonNoteHeaderCreator.hpp
        note/WhatSonNoteLinkManagerCreator.hpp
        note/WhatSonNoteAttachManagerCreator.hpp
)

if (../../platform/Android)
    set_property(TARGET WhatSon PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
            "../../platform/Android")
endif ()

if (../../platform/Apple/iOS)
    set_target_properties(WhatSon PROPERTIES
            MACOSX_BUNDLE_INFO_PLIST "../../platform/Apple/iOS/Info.plist"
    )
endif ()

qt_add_qml_module(WhatSon
        URI WhatSon.App
        VERSION 1.0
        RESOURCE_PREFIX "/qt/qml"
        QML_FILES
        qml/Main.qml
        qml/components/NavigationRail.qml
        qml/components/MetricCard.qml
        qml/components/InfoListCard.qml
        qml/components/InsightPanel.qml
        qml/pages/CreativeHubPage.qml
        qml/pages/BrandHubPage.qml
        qml/pages/KnowledgeHubPage.qml
        qml/pages/EditorStudioPage.qml
        qml/theme/DesignTokens.qml
)

target_link_libraries(WhatSon
        PRIVATE
        Qt6::Quick
        Qt6::QuickControls2
)

lvrs_configure_qml_app(WhatSon)

if (../../platform/Apple/iOS)
    # Exclude Qt qmltooling static plugins from iOS app linkage.
    # The installed Qt iOS kit may provide qmltooling init objects built for device arch,
    # which breaks simulator linking during bootstrap.
    if (COMMAND qt_import_plugins)
        qt_import_plugins(WhatSon EXCLUDE_BY_TYPE qmltooling)
    endif ()
endif ()

set_target_properties(WhatSon PROPERTIES
        MACOSX_BUNDLE ON
        OUTPUT_NAME "WhatSon"
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.lvrs.whatson"
        MACOSX_BUNDLE_BUNDLE_NAME "WhatSon"
)

# Custom launch targets for mobile platforms
find_program(WHATSON_XCRUN xcrun)
if (WHATSON_XCRUN)
    add_custom_target(launch_WhatSon_ios
            COMMAND "${CMAKE_COMMAND}" --build "${CMAKE_BINARY_DIR}" --target bootstrap_WhatSon_ios
            COMMAND "${WHATSON_XCRUN}" simctl launch booted com.lvrs.whatson
            USES_TERMINAL
            COMMENT "Launching WhatSon on iOS simulator"
    )
endif ()

find_program(WHATSON_ADB adb HINTS "$ENV{HOME}/Library/Android/sdk/platform-tools")
if (WHATSON_ADB)
    add_custom_target(launch_WhatSon_android
            COMMAND "${CMAKE_COMMAND}" --build "${CMAKE_BINARY_DIR}" --target bootstrap_WhatSon_android
            COMMAND "${WHATSON_ADB}" shell monkey -p com.lvrs.whatson -c android.intent.category.LAUNCHER 1
            USES_TERMINAL
            COMMENT "Launching WhatSon on Android device/emulator"
    )
endif ()
