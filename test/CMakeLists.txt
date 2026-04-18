find_package(Qt6 6.5 REQUIRED COMPONENTS Core Test Qml)

set(WHATSON_CPP_REGRESSION_TEST_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/cpp/whatson_cpp_regression_tests.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/platform/Android/WhatSonAndroidStorageBackend.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/policy/ArchitecturePolicyLock.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/store/hub/SelectedHubStore.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/store/sidebar/SidebarSelectionStore.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/runtime/scheduler/WhatSonAsyncScheduler.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/runtime/scheduler/WhatSonCronExpression.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/runtime/scheduler/WhatSonUnixTimeAnalyzer.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/viewmodel/content/ContentsResourceTagTextGenerator.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/viewmodel/navigationbar/EditorViewModeViewModel.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/viewmodel/navigationbar/EditorViewSectionViewModel.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/viewmodel/navigationbar/EditorViewState.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/viewmodel/navigationbar/NavigationModeSectionViewModel.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/viewmodel/navigationbar/NavigationModeState.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/viewmodel/navigationbar/NavigationModeViewModel.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/viewmodel/onboarding/OnboardingRouteBootstrapController.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/viewmodel/sidebar/IHierarchyViewModelProvider.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/viewmodel/sidebar/HierarchyViewModelProvider.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/viewmodel/sidebar/SidebarHierarchyViewModel.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/file/note/WhatSonNoteBodyResourceTagGenerator.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/viewmodel/content/ContentsStructuredDocumentCollectionPolicy.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/viewmodel/content/ContentsStructuredDocumentMutationPolicy.cpp"
)

add_executable(whatson_cpp_regression_tests
        ${WHATSON_CPP_REGRESSION_TEST_SOURCES}
)
target_include_directories(whatson_cpp_regression_tests PRIVATE
        "${CMAKE_SOURCE_DIR}/src/app"
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
