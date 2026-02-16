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
if (EXISTS "${WHATSON_IOS_INFO_PLIST}")
    list(APPEND _whatson_lvrs_defaults_args APPLE_INFO_PLIST "${WHATSON_IOS_INFO_PLIST}")
elseif (EXISTS "${WHATSON_DEFAULT_INFO_PLIST}")
    list(APPEND _whatson_lvrs_defaults_args APPLE_INFO_PLIST "${WHATSON_DEFAULT_INFO_PLIST}")
endif ()
if (EXISTS "${WHATSON_ENTITLEMENTS}")
    list(APPEND _whatson_lvrs_defaults_args APPLE_ENTITLEMENTS "${WHATSON_ENTITLEMENTS}")
endif ()

lvrs_configure_project_defaults(${_whatson_lvrs_defaults_args})

qt_add_qml_module(WhatSon
        URI WhatSon.App
        VERSION 1.0
        RESOURCE_PREFIX "/qt/qml"
        QML_FILES
        qml/Main.qml
        qml/components/layoutshell/StatusBarLayout.qml
        qml/components/layoutshell/NavigationBarLayout.qml
        qml/components/layoutshell/ContentViewLayout.qml
        qml/components/layoutshell/BodyLayout.qml
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

set_target_properties(WhatSon PROPERTIES
        MACOSX_BUNDLE ON
        OUTPUT_NAME "WhatSon"
        MACOSX_BUNDLE_BUNDLE_NAME "WhatSon"
)
