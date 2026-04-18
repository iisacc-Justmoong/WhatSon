find_package(Qt6 6.5 REQUIRED COMPONENTS Core Gui Test Qml)

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

set(WHATSON_CPP_REGRESSION_TEST_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/whatson_cpp_regression_tests.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/platform/Android/WhatSonAndroidStorageBackend.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/agenda/ContentsAgendaBackend.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/display/paper/ContentsA4PaperBackground.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/display/paper/ContentsPaperSelection.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/sensor/UnusedNoteSensorSupport.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/sensor/WeeklyUnusedNote.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/sensor/MonthlyUnusedNote.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/sensor/UnusedResourcesSensor.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/editor/parser/ContentsWsnBodyBlockParser.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/editor/renderer/ContentsHtmlBlockRenderPipeline.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/display/paper/ContentsTextFormatRenderer.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/display/paper/print/ContentsPagePrintLayoutRenderer.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/editor/renderer/ContentsTextHighlightRenderer.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/file/import/WhatSonClipboardResourceImportFileNamePolicy.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/file/IO/WhatSonSystemIoGateway.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/file/diff/WhatSonLocalNoteVersionStore.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/file/note/ContentsNoteManagementCoordinator.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/file/note/WhatSonLocalNoteFileStore.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/file/note/WhatSonNoteBodyPersistence.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/file/note/WhatSonNoteBodyWebLinkSupport.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/file/note/WhatSonNoteBodySemanticTagSupport.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/file/note/WhatSonNoteCreator.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/file/note/WhatSonNoteHeaderCreator.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/file/note/WhatSonNoteHeaderParser.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/file/note/WhatSonNoteHeaderStore.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/file/note/WhatSonNoteMarkdownStyleObject.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/policy/ArchitecturePolicyLock.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/file/statistic/WhatSonNoteFileStatSupport.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/file/sync/ContentsEditorIdleSyncController.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/folders/WhatSonFoldersHierarchyCreator.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/folders/WhatSonFoldersHierarchyParser.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/folders/WhatSonFoldersHierarchyStore.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/resources/WhatSonResourcesHierarchyCreator.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/resources/WhatSonResourcesHierarchyParser.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/resources/WhatSonResourcesHierarchyStore.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/tags/WhatSonTagsDepthFlattener.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/tags/WhatSonTagsHierarchyCreator.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/tags/WhatSonTagsHierarchyParser.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/tags/WhatSonTagsHierarchyStore.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/tags/WhatSonTagsJsonParser.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/file/validator/WhatSonStructuredTagLinter.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/file/viewer/ImageFormatCompatibilityLayer.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/file/viewer/ResourceBitmapViewer.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/store/hub/SelectedHubStore.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/store/sidebar/ISidebarSelectionStore.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/store/sidebar/ISidebarSelectionStore.hpp"
        "${CMAKE_SOURCE_DIR}/src/app/store/sidebar/SidebarSelectionStore.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/runtime/scheduler/WhatSonAsyncScheduler.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/runtime/scheduler/WhatSonCronExpression.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/runtime/scheduler/WhatSonUnixTimeAnalyzer.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/runtime/startup/WhatSonStartupHubResolver.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/viewmodel/hierarchy/IHierarchyViewModel.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/viewmodel/hierarchy/IHierarchyViewModel.hpp"
        "${CMAKE_SOURCE_DIR}/src/app/viewmodel/content/ContentsResourceTagTextGenerator.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/viewmodel/content/ContentsEditorSelectionBridge.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/viewmodel/content/ContentsEditorSessionController.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/viewmodel/content/ContentsLogicalTextBridge.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/viewmodel/content/ContentsStructuredDocumentBlocksModel.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/viewmodel/hierarchy/library/LibraryNoteListModel.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/viewmodel/hierarchy/resources/ResourcesHierarchyModel.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/viewmodel/hierarchy/resources/ResourcesHierarchyViewModel.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/viewmodel/hierarchy/resources/ResourcesListModel.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/viewmodel/navigationbar/EditorViewModeViewModel.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/viewmodel/navigationbar/EditorViewSectionViewModel.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/viewmodel/navigationbar/EditorViewState.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/viewmodel/navigationbar/NavigationModeSectionViewModel.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/viewmodel/navigationbar/NavigationModeState.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/viewmodel/navigationbar/NavigationModeViewModel.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/viewmodel/onboarding/IOnboardingHubController.hpp"
        "${CMAKE_SOURCE_DIR}/src/app/viewmodel/onboarding/OnboardingRouteBootstrapController.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/viewmodel/panel/NoteListModelContractBridge.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/viewmodel/sidebar/IActiveHierarchyContextSource.hpp"
        "${CMAKE_SOURCE_DIR}/src/app/viewmodel/sidebar/IActiveHierarchySource.hpp"
        "${CMAKE_SOURCE_DIR}/src/app/viewmodel/sidebar/IHierarchyViewModelProvider.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/viewmodel/sidebar/HierarchyViewModelProvider.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/viewmodel/sidebar/SidebarHierarchyViewModel.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/file/note/WhatSonNoteBodyResourceTagGenerator.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/viewmodel/content/ContentsStructuredDocumentCollectionPolicy.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/viewmodel/content/ContentsStructuredDocumentFocusPolicy.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/viewmodel/content/ContentsStructuredDocumentHost.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/viewmodel/content/ContentsStructuredDocumentMutationPolicy.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/viewmodel/detailPanel/session/WhatSonFoldersHierarchySessionService.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/viewmodel/detailPanel/ResourceDetailPanelViewModel.cpp"
)

add_executable(whatson_cpp_regression_tests
        ${WHATSON_CPP_REGRESSION_TEST_SOURCES}
)
target_include_directories(whatson_cpp_regression_tests PRIVATE
        "${CMAKE_SOURCE_DIR}/src/app"
        "${CMAKE_SOURCE_DIR}/src/app/models"
        "${CMAKE_SOURCE_DIR}/src/app/file"
)
target_link_libraries(whatson_cpp_regression_tests PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Test
        Qt6::Qml
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
