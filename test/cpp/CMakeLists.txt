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
        "${CMAKE_SOURCE_DIR}/src/app/models/sensor/UnusedNoteSensorSupport.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/sensor/WeeklyUnusedNote.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/sensor/MonthlyUnusedNote.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/sensor/UnusedResourcesSensor.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/content/mobile/MobileHierarchyRouteStateStore.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/content/mobile/MobileHierarchySelectionCoordinator.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/content/mobile/MobileHierarchyNavigationCoordinator.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/calendar/ISystemCalendarStore.hpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/calendar/SystemCalendarStore.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/clipboard/ClipboardResourceImport.h"
        "${CMAKE_SOURCE_DIR}/src/app/models/clipboard/ClipboardResourceImport.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/clipboard/FiletypeCapture.h"
        "${CMAKE_SOURCE_DIR}/src/app/models/clipboard/FiletypeCapture.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/clipboard/ClipboardEditorPaste.h"
        "${CMAKE_SOURCE_DIR}/src/app/models/clipboard/ClipboardEditorPaste.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/clipboard/InAppClipboardStore.h"
        "${CMAKE_SOURCE_DIR}/src/app/models/clipboard/InAppClipboardStore.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/clipboard/InAppClipboardManager.h"
        "${CMAKE_SOURCE_DIR}/src/app/models/clipboard/InAppClipboardManager.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/editor/GetProperty.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/editor/component/Break.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/editor/component/Callout.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/editor/component/ResourceImageFrame.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/editor/EditorInputCommandFilter.hpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/editor/EditorInputCommandFilter.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/editor/TagInsertionWriter.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/editor/NoteEditorDocumentSession.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/editor/SetProperty.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/editor/SetTag.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/file/diff/VersionLimitManager.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/file/conflict/WhatSonTimestampConflictResolver.hpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/file/conflict/WhatSonTimestampConflictResolver.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/file/diff/WhatSonLocalNoteVersionStore.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/file/diff/WhatSonNoteVersionDiffBuilder.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/file/diff/WhatSonNoteVersionFileGateway.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/file/diff/WhatSonNoteVersionSnapshotBuilder.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/file/diff/WhatSonNoteVersionStateCodec.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/file/hub/WhatSonHubPackager.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/file/hub/WhatSonHubStat.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/file/hub/WhatSonHubStore.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/file/sync/WhatSonHubSyncController.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/file/sync/WhatSonEditorRawPullController.hpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/file/sync/WhatSonEditorRawPullController.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/file/sync/WhatSonEditorRawPushController.hpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/file/sync/WhatSonEditorRawPushController.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/file/sync/WhatSonHubSyncObservationBuilder.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/file/sync/WhatSonHubSyncScheduler.hpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/file/sync/WhatSonHubSyncScheduler.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/file/sync/WhatSonHubSyncWatcher.hpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/file/sync/WhatSonHubSyncWatcher.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/file/note/session/ContentsNoteManagementCoordinator.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/file/note/hub/WhatSonHubNoteMutationSupport.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/file/note/local/WhatSonLocalNoteFileStore.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/file/note/body/WhatSonNoteBodyPersistence.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/file/note/body/WhatSonNoteBodyWebLinkSupport.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/file/note/body/WhatSonNoteBodySemanticTagSupport.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/file/note/support/WhatSonIiXmlDocumentSupport.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/file/note/package/WhatSonNoteCreator.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/file/note/header/WhatSonNoteHeaderCreator.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/file/note/header/WhatSonNoteHeaderParser.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/file/note/header/WhatSonNoteHeaderStore.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/file/note/body/WhatSonNoteMarkdownStyleObject.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/file/validator/WhatSonHubStructureValidator.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/file/validator/WhatSonLibraryIndexIntegrityValidator.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/file/validator/WhatSonNoteStorageValidator.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/policy/ArchitecturePolicyLock.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/file/statistic/WhatSonNoteFileStatSupport.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/hierarchy/folders/WhatSonFoldersHierarchyCreator.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/hierarchy/folders/WhatSonFoldersHierarchyParser.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/hierarchy/folders/WhatSonFoldersHierarchyStore.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/hierarchy/resources/WhatSonResourcesHierarchyCreator.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/hierarchy/resources/WhatSonResourcesHierarchyParser.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/hierarchy/resources/WhatSonResourcesHierarchyStore.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/hierarchy/projects/WhatSonProjectsHierarchyCreator.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/hierarchy/projects/WhatSonProjectsHierarchyParser.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/hierarchy/projects/WhatSonProjectsHierarchyStore.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/hierarchy/tags/WhatSonTagsDepthFlattener.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/hierarchy/tags/WhatSonTagsHierarchyCreator.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/hierarchy/tags/WhatSonTagsHierarchyParser.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/hierarchy/tags/WhatSonTagsHierarchyStore.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/hierarchy/tags/WhatSonTagsJsonParser.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/file/hub/WhatSonHubMountValidator.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/runtime/startup/WhatSonStartupHubResolver.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/store/hub/SelectedHubStore.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/store/sidebar/ISidebarSelectionStore.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/store/sidebar/ISidebarSelectionStore.hpp"
        "${CMAKE_SOURCE_DIR}/src/app/store/sidebar/SidebarSelectionStore.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/runtime/scheduler/WhatSonAsyncScheduler.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/runtime/scheduler/WhatSonCronExpression.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/runtime/scheduler/WhatSonUnixTimeAnalyzer.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/hierarchy/IHierarchyController.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/hierarchy/IHierarchyController.hpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/hierarchy/WhatSonHierarchyModel.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/hierarchy/WhatSonHierarchyNoteRecordSupport.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/hierarchy/library/LibraryAll.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/hierarchy/library/LibraryHierarchyController.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/hierarchy/library/LibraryDraft.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/hierarchy/library/LibraryNoteListModel.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/hierarchy/library/LibraryToday.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/hierarchy/library/WhatSonLibraryFolderHierarchyMutationService.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/hierarchy/library/WhatSonLibraryHierarchyCreator.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/hierarchy/library/WhatSonLibraryHierarchyParser.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/hierarchy/library/WhatSonLibraryHierarchyStore.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/hierarchy/library/WhatSonLibraryIndexedState.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/hierarchy/library/WhatSonLibraryNoteListProjection.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/hierarchy/resources/ResourcesHierarchyController.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/hierarchy/resources/ResourcesListModel.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/navigationbar/EditorViewSectionController.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/navigationbar/EditorViewState.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/navigationbar/EditorViewModeController.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/navigationbar/NavigationModeSectionController.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/navigationbar/NavigationModeState.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/navigationbar/NavigationModeController.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/onboarding/IOnboardingHubController.hpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/onboarding/OnboardingRouteBootstrapController.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/panel/HierarchyDragDropBridge.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/panel/HierarchyInteractionBridge.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/panel/NoteActiveStateTracker.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/panel/NoteListModelContractBridge.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/sidebar/IActiveHierarchyContextSource.hpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/sidebar/IActiveHierarchySource.hpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/sidebar/IHierarchyControllerProvider.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/sidebar/HierarchyControllerProvider.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/sidebar/SidebarHierarchyController.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/sidebar/SidebarHierarchyInteractionController.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/file/note/body/WhatSonNoteBodyResourceTagGenerator.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/file/note/folder/WhatSonNoteFolderBindingRepository.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/file/note/folder/WhatSonNoteFolderBindingService.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/file/note/hub/WhatSonHubNoteCreationService.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/file/note/hub/WhatSonHubNoteDeletionService.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/file/note/hub/WhatSonHubNoteFolderClearService.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/detailPanel/session/WhatSonFoldersHierarchySessionService.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/detailPanel/session/IWhatSonNoteHeaderSessionStore.hpp"
        "${CMAKE_SOURCE_DIR}/src/app/models/detailPanel/session/WhatSonNoteHeaderSessionStore.cpp"
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
