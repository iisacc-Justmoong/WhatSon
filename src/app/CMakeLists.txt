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

if (COMMAND whatson_configure_app_target)
    whatson_configure_app_target(WhatSon)
endif ()

if (COMMAND whatson_ensure_lvrs_plugin_target)
    whatson_ensure_lvrs_plugin_target()
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

if (NOT DEFINED WHATSON_APPLE_BUNDLE_ID)
    set(WHATSON_APPLE_BUNDLE_ID "com.lvrs.whatson")
endif ()

set_target_properties(WhatSon PROPERTIES
        MACOSX_BUNDLE ON
        OUTPUT_NAME "WhatSon"
        MACOSX_BUNDLE_GUI_IDENTIFIER "${WHATSON_APPLE_BUNDLE_ID}"
        MACOSX_BUNDLE_BUNDLE_NAME "WhatSon"
)
