find_package(Qt6 6.5 REQUIRED COMPONENTS Core Gui Test Qml Quick)

if (APPLE AND TARGET WrapOpenGL::WrapOpenGL)
    get_target_property(_whatson_test_wrap_opengl_links WrapOpenGL::WrapOpenGL INTERFACE_LINK_LIBRARIES)
    if (_whatson_test_wrap_opengl_links)
        set(_whatson_test_wrap_opengl_links_filtered)
        foreach (_whatson_test_wrap_link IN LISTS _whatson_test_wrap_opengl_links)
            if (_whatson_test_wrap_link STREQUAL "-framework AGL" OR _whatson_test_wrap_link MATCHES "AGL\\.framework")
                continue()
            endif ()
            list(APPEND _whatson_test_wrap_opengl_links_filtered "${_whatson_test_wrap_link}")
        endforeach ()
        set_property(TARGET WrapOpenGL::WrapOpenGL PROPERTY
                INTERFACE_LINK_LIBRARIES "${_whatson_test_wrap_opengl_links_filtered}")
    endif ()
endif ()

file(GLOB WHATSON_CPP_REGRESSION_SUITE_SOURCES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/suites/*.cpp")

set(WHATSON_CPP_REGRESSION_TEST_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/whatson_cpp_regression_tests.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/whatson_cpp_regression_tests_main.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/whatson_cpp_regression_test_support.cpp"
        ${WHATSON_CPP_REGRESSION_SUITE_SOURCES}
        "${CMAKE_SOURCE_DIR}/src/app/platform/Android/WhatSonAndroidStorageBackend.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/editor/tags/ContentsAgendaBackend.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/editor/tags/ContentsCalloutBackend.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/display/paper/ContentsA4PaperBackground.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/display/paper/ContentsPaperSelection.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/sensor/UnusedNoteSensorSupport.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/sensor/WeeklyUnusedNote.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/sensor/MonthlyUnusedNote.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/sensor/UnusedResourcesSensor.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/content/mobile/MobileHierarchyRouteStateStore.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/content/mobile/MobileHierarchySelectionCoordinator.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/content/mobile/MobileHierarchyNavigationCoordinator.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/editor/parser/ContentsWsnBodyBlockParser.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/editor/renderer/ContentsHtmlBlockRenderPipeline.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/editor/renderer/ContentsStructuredBlockRenderer.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/editor/tags/ContentsEditorTagInsertionController.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/editor/format/ContentsInlineStyleOverlayRenderer.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/editor/format/ContentsPlainTextSourceMutator.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/editor/format/ContentsTextFormatRenderer.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/editor/lineNumber/ContentsLineNumberRailMetrics.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/editor/minimap/ContentsEditorVisualLineMetrics.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/editor/minimap/ContentsMinimapLayoutMetrics.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/editor/format/ContentsTextFormatRendererEditorSurface.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/editor/format/ContentsTextFormatRendererMarkdown.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/display/paper/print/ContentsPagePrintLayoutRenderer.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/editor/format/ContentsTextHighlightRenderer.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/editor/projection/ContentsEditorPresentationProjection.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/editor/resource/ContentsEditorDynamicObjectSupport.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/editor/resource/ContentsInlineResourcePresentationController.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/editor/input/ContentsQmlBackedInputControllerBase.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/editor/input/ContentsEditorInputPolicyAdapter.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/editor/input/ContentsInlineFormatEditorController.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/file/import/WhatSonClipboardResourceImportFileNamePolicy.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/file/IO/WhatSonSystemIoGateway.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/file/diff/WhatSonLocalNoteVersionStore.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/file/hub/WhatSonHubPackager.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/file/note/ContentsNoteManagementCoordinator.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/file/note/WhatSonLocalNoteFileStore.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/file/note/WhatSonNoteBodyPersistence.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/file/note/WhatSonNoteBodyWebLinkSupport.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/file/note/WhatSonNoteBodySemanticTagSupport.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/file/note/WhatSonIiXmlDocumentSupport.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/file/note/WhatSonNoteCreator.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/file/note/WhatSonNoteHeaderCreator.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/file/note/WhatSonNoteHeaderParser.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/file/note/WhatSonNoteHeaderStore.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/file/note/WhatSonNoteMarkdownStyleObject.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/policy/ArchitecturePolicyLock.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/file/statistic/WhatSonNoteFileStatSupport.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/editor/persistence/ContentsEditorPersistenceController.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/file/hierarchy/folders/WhatSonFoldersHierarchyCreator.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/file/hierarchy/folders/WhatSonFoldersHierarchyParser.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/file/hierarchy/folders/WhatSonFoldersHierarchyStore.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/file/hierarchy/resources/WhatSonResourcesHierarchyCreator.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/file/hierarchy/resources/WhatSonResourcesHierarchyParser.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/file/hierarchy/resources/WhatSonResourcesHierarchyStore.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/file/hierarchy/tags/WhatSonTagsDepthFlattener.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/file/hierarchy/tags/WhatSonTagsHierarchyCreator.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/file/hierarchy/tags/WhatSonTagsHierarchyParser.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/file/hierarchy/tags/WhatSonTagsHierarchyStore.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/file/hierarchy/tags/WhatSonTagsJsonParser.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/editor/tags/WhatSonStructuredTagLinter.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/editor/tags/ContentsResourceTagController.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/file/viewer/ImageFormatCompatibilityLayer.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/file/viewer/ContentsBodyResourceRenderer.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/file/viewer/ResourceBitmapViewer.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/file/hub/WhatSonHubMountValidator.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/runtime/startup/WhatSonStartupHubResolver.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/store/hub/SelectedHubStore.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/store/sidebar/ISidebarSelectionStore.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/store/sidebar/ISidebarSelectionStore.hpp"
        "${CMAKE_SOURCE_DIR}/src/app/store/sidebar/SidebarSelectionStore.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/runtime/scheduler/WhatSonAsyncScheduler.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/runtime/scheduler/WhatSonCronExpression.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/runtime/scheduler/WhatSonUnixTimeAnalyzer.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/file/hierarchy/IHierarchyController.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/file/hierarchy/IHierarchyController.hpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/editor/tags/ContentsResourceTagTextGenerator.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/editor/bridge/ContentsEditorSelectionBridge.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/editor/session/ContentsEditorSessionController.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/editor/text/ContentsLogicalTextBridge.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/editor/structure/ContentsStructuredDocumentBlocksModel.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/file/hierarchy/library/LibraryNoteListModel.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/file/hierarchy/resources/ResourcesHierarchyModel.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/file/hierarchy/resources/ResourcesHierarchyController.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/file/hierarchy/resources/ResourcesListModel.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/navigationbar/EditorViewModeController.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/navigationbar/EditorViewSectionController.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/navigationbar/EditorViewState.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/navigationbar/NavigationModeSectionController.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/navigationbar/NavigationModeState.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/navigationbar/NavigationModeController.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/onboarding/IOnboardingHubController.hpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/onboarding/OnboardingRouteBootstrapController.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/panel/NoteActiveStateTracker.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/panel/NoteListModelContractBridge.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/sidebar/IActiveHierarchyContextSource.hpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/sidebar/IActiveHierarchySource.hpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/sidebar/IHierarchyControllerProvider.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/sidebar/HierarchyControllerProvider.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/sidebar/SidebarHierarchyController.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/file/note/WhatSonNoteBodyResourceTagGenerator.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/editor/structure/ContentsStructuredDocumentCollectionPolicy.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/editor/structure/ContentsStructuredDocumentFocusPolicy.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/editor/structure/ContentsStructuredDocumentHost.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/editor/structure/ContentsStructuredDocumentMutationPolicy.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/detailPanel/session/WhatSonFoldersHierarchySessionService.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/detailPanel/DetailCurrentNoteContextBridge.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/detailPanel/ResourceDetailPanelController.cpp"
)

if (APPLE)
    list(APPEND WHATSON_CPP_REGRESSION_TEST_SOURCES
            "${CMAKE_SOURCE_DIR}/src/app/platform/Apple/WhatSonApplePackageAppearance.mm"
    )
endif ()

add_executable(whatson_cpp_regression_tests
        ${WHATSON_CPP_REGRESSION_TEST_SOURCES}
)
target_include_directories(whatson_cpp_regression_tests PRIVATE
        "${CMAKE_SOURCE_DIR}"
        "${CMAKE_SOURCE_DIR}/src"
)
target_link_libraries(whatson_cpp_regression_tests PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Test
        Qt6::Qml
        Qt6::Quick
        iiXml::iiXml
        iiHtmlBlock::iiHtmlBlock
)

add_test(
        NAME whatson_cpp_regression
        COMMAND whatson_cpp_regression_tests
)
set_tests_properties(
        whatson_cpp_regression
        PROPERTIES
        LABELS "cpp_regression"
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
)

add_custom_target(whatson_cpp_regression
        COMMAND "${CMAKE_CTEST_COMMAND}" --output-on-failure -L cpp_regression
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
        USES_TERMINAL
        COMMENT "Run WhatSon C++ regression tests"
)
