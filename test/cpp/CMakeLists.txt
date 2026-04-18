find_package(Qt6 6.5 REQUIRED COMPONENTS Core Test Qml)

set(WHATSON_CPP_REGRESSION_TEST_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/whatson_cpp_regression_tests.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/platform/Android/WhatSonAndroidStorageBackend.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/agenda/ContentsAgendaBackend.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/file/import/WhatSonClipboardResourceImportFileNamePolicy.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/file/IO/WhatSonSystemIoGateway.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/file/diff/WhatSonLocalNoteVersionStore.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/file/note/ContentsNoteManagementCoordinator.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/file/note/WhatSonLocalNoteFileStore.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/file/note/WhatSonNoteBodyPersistence.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/file/note/WhatSonNoteBodySemanticTagSupport.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/file/note/WhatSonNoteCreator.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/file/note/WhatSonNoteHeaderCreator.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/file/note/WhatSonNoteHeaderParser.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/file/note/WhatSonNoteHeaderStore.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/file/note/WhatSonNoteMarkdownStyleObject.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/policy/ArchitecturePolicyLock.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/file/statistic/WhatSonNoteFileStatSupport.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/file/sync/ContentsEditorIdleSyncController.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/tags/WhatSonTagsDepthFlattener.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/tags/WhatSonTagsHierarchyCreator.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/tags/WhatSonTagsHierarchyParser.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/tags/WhatSonTagsHierarchyStore.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/tags/WhatSonTagsJsonParser.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/file/validator/WhatSonStructuredTagLinter.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/store/hub/SelectedHubStore.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/store/sidebar/ISidebarSelectionStore.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/store/sidebar/ISidebarSelectionStore.hpp"
        "${CMAKE_SOURCE_DIR}/src/app/store/sidebar/SidebarSelectionStore.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/runtime/scheduler/WhatSonAsyncScheduler.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/runtime/scheduler/WhatSonCronExpression.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/runtime/scheduler/WhatSonUnixTimeAnalyzer.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/viewmodel/hierarchy/IHierarchyViewModel.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/viewmodel/hierarchy/IHierarchyViewModel.hpp"
        "${CMAKE_SOURCE_DIR}/src/app/viewmodel/content/ContentsResourceTagTextGenerator.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/viewmodel/content/ContentsEditorSelectionBridge.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/viewmodel/content/ContentsEditorSessionController.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/viewmodel/content/ContentsStructuredDocumentBlocksModel.cpp"
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
)

add_executable(whatson_cpp_regression_tests
        ${WHATSON_CPP_REGRESSION_TEST_SOURCES}
)
target_include_directories(whatson_cpp_regression_tests PRIVATE
        "${CMAKE_SOURCE_DIR}/src/app"
        "${CMAKE_SOURCE_DIR}/src/app/file"
)
target_link_libraries(whatson_cpp_regression_tests PRIVATE
        Qt6::Core
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
