find_package(Qt6 6.5 REQUIRED COMPONENTS Core Test)

set(WHATSON_TEST_TARGETS)

add_executable(whatson_hub_creator_tests
        app/test_whatson_workspace_hub_creator.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hub/WhatSonHubCreator.cpp
)
set_target_properties(whatson_hub_creator_tests PROPERTIES AUTOMOC ON)
target_include_directories(whatson_hub_creator_tests
        PRIVATE
        ${CMAKE_SOURCE_DIR}/src/app/file
        ${CMAKE_SOURCE_DIR}/src/app/file/hub
)
target_link_libraries(whatson_hub_creator_tests
        PRIVATE
        Qt6::Core
        Qt6::Test
)
add_test(NAME whatson_hub_creator_tests COMMAND whatson_hub_creator_tests)
list(APPEND WHATSON_TEST_TARGETS whatson_hub_creator_tests)

add_executable(whatson_hub_parser_tests
        app/test_whatson_hub_parser.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hub/WhatSonHubParser.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hub/WhatSonHubStore.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hub/WhatSonHubStat.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/library/WhatSonLibraryHierarchyStore.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/library/WhatSonLibraryHierarchyParser.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/projects/WhatSonProjectsHierarchyStore.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/projects/WhatSonProjectsHierarchyParser.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/projects/WhatSonProjectsHierarchyCreator.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/bookmarks/WhatSonBookmarksHierarchyStore.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/bookmarks/WhatSonBookmarksHierarchyParser.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/progress/WhatSonProgressHierarchyStore.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/progress/WhatSonProgressHierarchyParser.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/progress/WhatSonProgressHierarchyCreator.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/event/WhatSonEventHierarchyStore.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/event/WhatSonEventHierarchyParser.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/event/WhatSonEventHierarchyCreator.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/preset/WhatSonPresetHierarchyStore.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/preset/WhatSonPresetHierarchyParser.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/preset/WhatSonPresetHierarchyCreator.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/tags/WhatSonTagsHierarchyStore.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/tags/WhatSonTagsHierarchyParser.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/tags/WhatSonTagsHierarchyCreator.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/tags/WhatSonTagsJsonParser.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/tags/WhatSonTagsDepthFlattener.cpp
)
set_target_properties(whatson_hub_parser_tests PROPERTIES AUTOMOC ON)
target_include_directories(whatson_hub_parser_tests
        PRIVATE
        ${CMAKE_SOURCE_DIR}/src/app/file
        ${CMAKE_SOURCE_DIR}/src/app/file/hub
)
target_link_libraries(whatson_hub_parser_tests
        PRIVATE
        Qt6::Core
        Qt6::Test
)
add_test(NAME whatson_hub_parser_tests COMMAND whatson_hub_parser_tests)
list(APPEND WHATSON_TEST_TARGETS whatson_hub_parser_tests)

add_executable(whatson_hub_placement_store_tests
        app/test_whatson_hub_placement_store.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hub/WhatSonHubPlacement.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hub/WhatSonHubPlacementStore.cpp
)
set_target_properties(whatson_hub_placement_store_tests PROPERTIES AUTOMOC ON)
target_include_directories(whatson_hub_placement_store_tests
        PRIVATE
        ${CMAKE_SOURCE_DIR}/src/app/file
        ${CMAKE_SOURCE_DIR}/src/app/file/hub
)
target_link_libraries(whatson_hub_placement_store_tests
        PRIVATE
        Qt6::Core
        Qt6::Test
)
add_test(NAME whatson_hub_placement_store_tests COMMAND whatson_hub_placement_store_tests)
list(APPEND WHATSON_TEST_TARGETS whatson_hub_placement_store_tests)

add_executable(whatson_hub_tags_state_store_tests
        app/test_whatson_hub_tags_state_store.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/tags/WhatSonHubTagsStateStore.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/tags/WhatSonHubTagsDepthProvider.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/tags/WhatSonHubTagsPathResolver.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/tags/WhatSonTagsFileReader.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/tags/WhatSonTagsJsonParser.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/tags/WhatSonTagsDepthFlattener.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/note/WhatSonNoteHeaderStore.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/note/WhatSonNoteHeaderParser.cpp
)
set_target_properties(whatson_hub_tags_state_store_tests PROPERTIES AUTOMOC ON)
target_include_directories(whatson_hub_tags_state_store_tests
        PRIVATE
        ${CMAKE_SOURCE_DIR}/src/app/file
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/tags
)
target_link_libraries(whatson_hub_tags_state_store_tests
        PRIVATE
        Qt6::Core
        Qt6::Test
)
add_test(NAME whatson_hub_tags_state_store_tests COMMAND whatson_hub_tags_state_store_tests)
list(APPEND WHATSON_TEST_TARGETS whatson_hub_tags_state_store_tests)

add_executable(whatson_hub_runtime_store_tests
        app/test_whatson_hub_runtime_store.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hub/WhatSonHubParser.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hub/WhatSonHubStore.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hub/WhatSonHubStat.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/library/WhatSonLibraryHierarchyStore.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/library/WhatSonLibraryHierarchyParser.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/projects/WhatSonProjectsHierarchyStore.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/projects/WhatSonProjectsHierarchyParser.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/projects/WhatSonProjectsHierarchyCreator.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/bookmarks/WhatSonBookmarksHierarchyStore.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/bookmarks/WhatSonBookmarksHierarchyParser.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/progress/WhatSonProgressHierarchyStore.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/progress/WhatSonProgressHierarchyParser.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/progress/WhatSonProgressHierarchyCreator.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/event/WhatSonEventHierarchyStore.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/event/WhatSonEventHierarchyParser.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/event/WhatSonEventHierarchyCreator.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/preset/WhatSonPresetHierarchyStore.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/preset/WhatSonPresetHierarchyParser.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/preset/WhatSonPresetHierarchyCreator.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/tags/WhatSonTagsHierarchyStore.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/tags/WhatSonTagsHierarchyParser.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/tags/WhatSonTagsHierarchyCreator.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hub/WhatSonHubPlacement.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hub/WhatSonHubPlacementStore.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hub/WhatSonHubRuntimeStore.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/tags/WhatSonHubTagsStateStore.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/tags/WhatSonHubTagsDepthProvider.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/tags/WhatSonHubTagsPathResolver.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/tags/WhatSonTagsFileReader.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/tags/WhatSonTagsJsonParser.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/tags/WhatSonTagsDepthFlattener.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/note/WhatSonNoteHeaderStore.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/note/WhatSonNoteHeaderParser.cpp
)
set_target_properties(whatson_hub_runtime_store_tests PROPERTIES AUTOMOC ON)
target_include_directories(whatson_hub_runtime_store_tests
        PRIVATE
        ${CMAKE_SOURCE_DIR}/src/app/file
        ${CMAKE_SOURCE_DIR}/src/app/file/hub
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/tags
)
target_link_libraries(whatson_hub_runtime_store_tests
        PRIVATE
        Qt6::Core
        Qt6::Test
)
add_test(NAME whatson_hub_runtime_store_tests COMMAND whatson_hub_runtime_store_tests)
list(APPEND WHATSON_TEST_TARGETS whatson_hub_runtime_store_tests)

add_executable(whatson_hub_tags_depth_provider_tests
        app/test_whatson_hub_tags_depth_provider.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/tags/WhatSonHubTagsDepthProvider.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/tags/WhatSonHubTagsPathResolver.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/tags/WhatSonTagsFileReader.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/tags/WhatSonTagsJsonParser.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/tags/WhatSonTagsDepthFlattener.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/note/WhatSonNoteHeaderStore.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/note/WhatSonNoteHeaderParser.cpp
)
set_target_properties(whatson_hub_tags_depth_provider_tests PROPERTIES AUTOMOC ON)
target_include_directories(whatson_hub_tags_depth_provider_tests
        PRIVATE
        ${CMAKE_SOURCE_DIR}/src/app/file
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/tags
)
target_link_libraries(whatson_hub_tags_depth_provider_tests
        PRIVATE
        Qt6::Core
        Qt6::Test
)
add_test(NAME whatson_hub_tags_depth_provider_tests COMMAND whatson_hub_tags_depth_provider_tests)
list(APPEND WHATSON_TEST_TARGETS whatson_hub_tags_depth_provider_tests)

add_executable(whatson_hierarchy_viewmodels_tests
        app/test_hierarchy_viewmodels.cpp
        ${CMAKE_SOURCE_DIR}/src/app/calendar/SystemCalendarStore.cpp
        ${CMAKE_SOURCE_DIR}/src/app/viewmodel/hierarchy/projects/ProjectsHierarchyModel.cpp
        ${CMAKE_SOURCE_DIR}/src/app/viewmodel/hierarchy/bookmarks/BookmarksHierarchyModel.cpp
        ${CMAKE_SOURCE_DIR}/src/app/viewmodel/hierarchy/bookmarks/BookmarksNoteListModel.cpp
        ${CMAKE_SOURCE_DIR}/src/app/viewmodel/hierarchy/resources/ResourcesHierarchyModel.cpp
        ${CMAKE_SOURCE_DIR}/src/app/viewmodel/hierarchy/progress/ProgressHierarchyModel.cpp
        ${CMAKE_SOURCE_DIR}/src/app/viewmodel/hierarchy/event/EventHierarchyModel.cpp
        ${CMAKE_SOURCE_DIR}/src/app/viewmodel/hierarchy/preset/PresetHierarchyModel.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/library/LibraryAll.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/library/LibraryDraft.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/library/LibraryToday.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/library/WhatSonLibraryHierarchyStore.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/library/WhatSonLibraryHierarchyCreator.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hub/WhatSonHubStore.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hub/WhatSonHubStat.cpp
        ${CMAKE_SOURCE_DIR}/src/app/viewmodel/hierarchy/library/LibraryHierarchyModel.cpp
        ${CMAKE_SOURCE_DIR}/src/app/viewmodel/hierarchy/library/LibraryNoteListModel.cpp
        ${CMAKE_SOURCE_DIR}/src/app/viewmodel/hierarchy/library/LibraryHierarchyViewModel.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/note/WhatSonNoteHeaderStore.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/note/WhatSonNoteHeaderParser.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/note/WhatSonNoteCreator.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/note/WhatSonNoteBodyCreator.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/note/WhatSonNoteLinkManagerCreator.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/note/WhatSonNoteAttachManagerCreator.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/note/WhatSonNoteBodyPersistence.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/note/WhatSonNoteHeaderCreator.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/projects/WhatSonProjectsHierarchyStore.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/projects/WhatSonProjectsHierarchyParser.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/projects/WhatSonProjectsHierarchyCreator.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/bookmarks/WhatSonBookmarksHierarchyStore.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/bookmarks/WhatSonBookmarksHierarchyParser.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/resources/WhatSonResourcesHierarchyStore.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/resources/WhatSonResourcesHierarchyParser.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/resources/WhatSonResourcesHierarchyCreator.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/progress/WhatSonProgressHierarchyStore.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/progress/WhatSonProgressHierarchyParser.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/progress/WhatSonProgressHierarchyCreator.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/event/WhatSonEventHierarchyStore.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/event/WhatSonEventHierarchyParser.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/event/WhatSonEventHierarchyCreator.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/preset/WhatSonPresetHierarchyStore.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/preset/WhatSonPresetHierarchyParser.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/preset/WhatSonPresetHierarchyCreator.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/tags/WhatSonTagsHierarchyStore.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/tags/WhatSonTagsHierarchyParser.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/tags/WhatSonTagsJsonParser.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/tags/WhatSonTagsDepthFlattener.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/tags/WhatSonTagsHierarchyCreator.cpp
        ${CMAKE_SOURCE_DIR}/src/app/viewmodel/hierarchy/tags/TagsHierarchyModel.cpp
        ${CMAKE_SOURCE_DIR}/src/app/viewmodel/hierarchy/tags/TagsHierarchyViewModel.cpp
        ${CMAKE_SOURCE_DIR}/src/app/viewmodel/hierarchy/projects/ProjectsHierarchyViewModel.cpp
        ${CMAKE_SOURCE_DIR}/src/app/viewmodel/hierarchy/bookmarks/BookmarksHierarchyViewModel.cpp
        ${CMAKE_SOURCE_DIR}/src/app/viewmodel/hierarchy/resources/ResourcesHierarchyViewModel.cpp
        ${CMAKE_SOURCE_DIR}/src/app/viewmodel/hierarchy/progress/ProgressHierarchyViewModel.cpp
        ${CMAKE_SOURCE_DIR}/src/app/viewmodel/hierarchy/event/EventHierarchyViewModel.cpp
        ${CMAKE_SOURCE_DIR}/src/app/viewmodel/hierarchy/preset/PresetHierarchyViewModel.cpp
)
set_target_properties(whatson_hierarchy_viewmodels_tests PROPERTIES AUTOMOC ON)
target_include_directories(whatson_hierarchy_viewmodels_tests
        PRIVATE
        ${CMAKE_SOURCE_DIR}/src/app
        ${CMAKE_SOURCE_DIR}/src/app/file
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/library
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/tags
        ${CMAKE_SOURCE_DIR}/src/app/viewmodel
)
target_link_libraries(whatson_hierarchy_viewmodels_tests
        PRIVATE
        Qt6::Core
        Qt6::Test
)
add_test(NAME whatson_hierarchy_viewmodels_tests COMMAND whatson_hierarchy_viewmodels_tests)
list(APPEND WHATSON_TEST_TARGETS whatson_hierarchy_viewmodels_tests)

add_executable(whatson_library_hierarchy_tests
        app/test_library_hierarchy_view_model.cpp
        ${CMAKE_SOURCE_DIR}/src/app/calendar/SystemCalendarStore.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/library/LibraryAll.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/library/LibraryDraft.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/library/LibraryToday.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/library/WhatSonLibraryHierarchyStore.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/library/WhatSonLibraryHierarchyCreator.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hub/WhatSonHubStore.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hub/WhatSonHubStat.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/projects/WhatSonProjectsHierarchyStore.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/projects/WhatSonProjectsHierarchyParser.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/projects/WhatSonProjectsHierarchyCreator.cpp
        ${CMAKE_SOURCE_DIR}/src/app/viewmodel/hierarchy/library/LibraryHierarchyModel.cpp
        ${CMAKE_SOURCE_DIR}/src/app/viewmodel/hierarchy/library/LibraryNoteListModel.cpp
        ${CMAKE_SOURCE_DIR}/src/app/viewmodel/hierarchy/library/LibraryHierarchyViewModel.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/note/WhatSonNoteHeaderStore.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/note/WhatSonNoteHeaderParser.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/note/WhatSonNoteCreator.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/note/WhatSonNoteBodyCreator.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/note/WhatSonNoteLinkManagerCreator.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/note/WhatSonNoteAttachManagerCreator.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/note/WhatSonNoteBodyPersistence.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/note/WhatSonNoteHeaderCreator.cpp
)
set_target_properties(whatson_library_hierarchy_tests PROPERTIES AUTOMOC ON)
target_include_directories(whatson_library_hierarchy_tests
        PRIVATE
        ${CMAKE_SOURCE_DIR}/src/app
        ${CMAKE_SOURCE_DIR}/src/app/file
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/library
        ${CMAKE_SOURCE_DIR}/src/app/viewmodel/hierarchy/library
)
target_link_libraries(whatson_library_hierarchy_tests
        PRIVATE
        Qt6::Core
        Qt6::Test
)
add_test(NAME whatson_library_hierarchy_tests COMMAND whatson_library_hierarchy_tests)
list(APPEND WHATSON_TEST_TARGETS whatson_library_hierarchy_tests)

add_executable(whatson_tags_hierarchy_tests
        app/test_tags_hierarchy_view_model.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/tags/WhatSonTagsHierarchyStore.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/tags/WhatSonTagsHierarchyParser.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/tags/WhatSonTagsJsonParser.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/tags/WhatSonTagsDepthFlattener.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/tags/WhatSonTagsHierarchyCreator.cpp
        ${CMAKE_SOURCE_DIR}/src/app/viewmodel/hierarchy/tags/TagsHierarchyModel.cpp
        ${CMAKE_SOURCE_DIR}/src/app/viewmodel/hierarchy/tags/TagsHierarchyViewModel.cpp
)
set_target_properties(whatson_tags_hierarchy_tests PROPERTIES AUTOMOC ON)
target_include_directories(whatson_tags_hierarchy_tests
        PRIVATE
        ${CMAKE_SOURCE_DIR}/src/app
        ${CMAKE_SOURCE_DIR}/src/app/file
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/tags
        ${CMAKE_SOURCE_DIR}/src/app/viewmodel/hierarchy/tags
)
target_link_libraries(whatson_tags_hierarchy_tests
        PRIVATE
        Qt6::Core
        Qt6::Test
)
add_test(NAME whatson_tags_hierarchy_tests COMMAND whatson_tags_hierarchy_tests)
list(APPEND WHATSON_TEST_TARGETS whatson_tags_hierarchy_tests)

add_executable(whatson_async_scheduler_tests
        app/test_async_scheduler.cpp
        ${CMAKE_SOURCE_DIR}/src/app/runtime/scheduler/WhatSonAsyncScheduler.cpp
        ${CMAKE_SOURCE_DIR}/src/app/runtime/scheduler/WhatSonCronExpression.cpp
        ${CMAKE_SOURCE_DIR}/src/app/runtime/scheduler/WhatSonUnixTimeAnalyzer.cpp
)
set_target_properties(whatson_async_scheduler_tests PROPERTIES AUTOMOC ON)
target_include_directories(whatson_async_scheduler_tests
        PRIVATE
        ${CMAKE_SOURCE_DIR}/src/app
)
target_link_libraries(whatson_async_scheduler_tests
        PRIVATE
        Qt6::Core
        Qt6::Test
)
add_test(NAME whatson_async_scheduler_tests COMMAND whatson_async_scheduler_tests)
list(APPEND WHATSON_TEST_TARGETS whatson_async_scheduler_tests)

add_executable(whatson_year_calendar_viewmodel_tests
        app/test_year_calendar_viewmodel.cpp
        ${CMAKE_SOURCE_DIR}/src/app/viewmodel/calendar/YearCalendarViewModel.cpp
)
set_target_properties(whatson_year_calendar_viewmodel_tests PROPERTIES AUTOMOC ON)
target_include_directories(whatson_year_calendar_viewmodel_tests
        PRIVATE
        ${CMAKE_SOURCE_DIR}/src/app
)
target_link_libraries(whatson_year_calendar_viewmodel_tests
        PRIVATE
        Qt6::Core
        Qt6::Test
)
add_test(NAME whatson_year_calendar_viewmodel_tests COMMAND whatson_year_calendar_viewmodel_tests)
list(APPEND WHATSON_TEST_TARGETS whatson_year_calendar_viewmodel_tests)

add_executable(whatson_system_calendar_store_tests
        app/test_system_calendar_store.cpp
        ${CMAKE_SOURCE_DIR}/src/app/calendar/SystemCalendarStore.cpp
)
set_target_properties(whatson_system_calendar_store_tests PROPERTIES AUTOMOC ON)
target_include_directories(whatson_system_calendar_store_tests
        PRIVATE
        ${CMAKE_SOURCE_DIR}/src/app
)
target_link_libraries(whatson_system_calendar_store_tests
        PRIVATE
        Qt6::Core
        Qt6::Test
)
add_test(NAME whatson_system_calendar_store_tests COMMAND whatson_system_calendar_store_tests)
list(APPEND WHATSON_TEST_TARGETS whatson_system_calendar_store_tests)

add_executable(whatson_qml_binding_guard_tests
        app/test_qml_binding_syntax_guard.cpp
)
set_target_properties(whatson_qml_binding_guard_tests PROPERTIES AUTOMOC ON)
target_link_libraries(whatson_qml_binding_guard_tests
        PRIVATE
        Qt6::Core
        Qt6::Test
)
add_test(NAME whatson_qml_binding_guard_tests COMMAND whatson_qml_binding_guard_tests)
list(APPEND WHATSON_TEST_TARGETS whatson_qml_binding_guard_tests)

add_executable(whatson_panel_viewmodel_registry_tests
        app/test_panel_viewmodel_registry.cpp
        ${CMAKE_SOURCE_DIR}/src/app/viewmodel/panel/PanelViewModel.cpp
        ${CMAKE_SOURCE_DIR}/src/app/viewmodel/panel/PanelViewModelRegistry.cpp
)
set_target_properties(whatson_panel_viewmodel_registry_tests PROPERTIES AUTOMOC ON)
target_include_directories(whatson_panel_viewmodel_registry_tests
        PRIVATE
        ${CMAKE_SOURCE_DIR}/src/app
)
target_link_libraries(whatson_panel_viewmodel_registry_tests
        PRIVATE
        Qt6::Core
        Qt6::Test
)
add_test(NAME whatson_panel_viewmodel_registry_tests COMMAND whatson_panel_viewmodel_registry_tests)
list(APPEND WHATSON_TEST_TARGETS whatson_panel_viewmodel_registry_tests)

add_executable(whatson_navigation_qml_frames_tests
        app/test_navigation_qml_frames.cpp
        ${CMAKE_SOURCE_DIR}/src/app/viewmodel/panel/PanelViewModel.cpp
        ${CMAKE_SOURCE_DIR}/src/app/viewmodel/panel/PanelViewModelRegistry.cpp
)
set_target_properties(whatson_navigation_qml_frames_tests PROPERTIES AUTOMOC ON)
target_include_directories(whatson_navigation_qml_frames_tests
        PRIVATE
        ${CMAKE_SOURCE_DIR}/src/app
)
target_link_libraries(whatson_navigation_qml_frames_tests
        PRIVATE
        Qt6::Core
        Qt6::Test
)
add_test(NAME whatson_navigation_qml_frames_tests COMMAND whatson_navigation_qml_frames_tests)
list(APPEND WHATSON_TEST_TARGETS whatson_navigation_qml_frames_tests)

add_executable(whatson_architecture_policy_lock_tests
        app/test_architecture_policy_lock.cpp
        ${CMAKE_SOURCE_DIR}/src/app/policy/ArchitecturePolicyLock.cpp
        ${CMAKE_SOURCE_DIR}/src/app/store/sidebar/ISidebarSelectionStore.cpp
        ${CMAKE_SOURCE_DIR}/src/app/store/sidebar/SidebarSelectionStore.cpp
        ${CMAKE_SOURCE_DIR}/src/app/viewmodel/sidebar/IHierarchyViewModelProvider.cpp
        ${CMAKE_SOURCE_DIR}/src/app/viewmodel/sidebar/HierarchyViewModelProvider.cpp
        ${CMAKE_SOURCE_DIR}/src/app/viewmodel/sidebar/SidebarHierarchyViewModel.cpp
)
set_target_properties(whatson_architecture_policy_lock_tests PROPERTIES AUTOMOC ON)
target_include_directories(whatson_architecture_policy_lock_tests
        PRIVATE
        ${CMAKE_SOURCE_DIR}/src/app
)
target_link_libraries(whatson_architecture_policy_lock_tests
        PRIVATE
        Qt6::Core
        Qt6::Test
)
add_test(NAME whatson_architecture_policy_lock_tests COMMAND whatson_architecture_policy_lock_tests)
list(APPEND WHATSON_TEST_TARGETS whatson_architecture_policy_lock_tests)

add_executable(whatson_sidebar_hierarchy_module_tests
        app/test_sidebar_hierarchy_module.cpp
        ${CMAKE_SOURCE_DIR}/src/app/policy/ArchitecturePolicyLock.cpp
        ${CMAKE_SOURCE_DIR}/src/app/store/sidebar/ISidebarSelectionStore.cpp
        ${CMAKE_SOURCE_DIR}/src/app/store/sidebar/SidebarSelectionStore.cpp
        ${CMAKE_SOURCE_DIR}/src/app/viewmodel/sidebar/IHierarchyViewModelProvider.cpp
        ${CMAKE_SOURCE_DIR}/src/app/viewmodel/sidebar/HierarchyViewModelProvider.cpp
        ${CMAKE_SOURCE_DIR}/src/app/viewmodel/sidebar/SidebarHierarchyViewModel.cpp
)
set_target_properties(whatson_sidebar_hierarchy_module_tests PROPERTIES AUTOMOC ON)
target_include_directories(whatson_sidebar_hierarchy_module_tests
        PRIVATE
        ${CMAKE_SOURCE_DIR}/src/app
)
target_link_libraries(whatson_sidebar_hierarchy_module_tests
        PRIVATE
        Qt6::Core
        Qt6::Test
)
add_test(NAME whatson_sidebar_hierarchy_module_tests COMMAND whatson_sidebar_hierarchy_module_tests)
list(APPEND WHATSON_TEST_TARGETS whatson_sidebar_hierarchy_module_tests)

add_executable(whatson_detail_panel_viewmodel_tests
        app/test_detail_panel_viewmodel.cpp
        ${CMAKE_SOURCE_DIR}/src/app/viewmodel/detailPanel/DetailContentSectionViewModel.cpp
        ${CMAKE_SOURCE_DIR}/src/app/viewmodel/detailPanel/DetailPanelState.cpp
        ${CMAKE_SOURCE_DIR}/src/app/viewmodel/detailPanel/DetailPanelToolbarItemsFactory.cpp
        ${CMAKE_SOURCE_DIR}/src/app/viewmodel/detailPanel/DetailPanelViewModel.cpp
)
set_target_properties(whatson_detail_panel_viewmodel_tests PROPERTIES AUTOMOC ON)
target_include_directories(whatson_detail_panel_viewmodel_tests
        PRIVATE
        ${CMAKE_SOURCE_DIR}/src/app
)
target_link_libraries(whatson_detail_panel_viewmodel_tests
        PRIVATE
        Qt6::Core
        Qt6::Test
)
add_test(NAME whatson_detail_panel_viewmodel_tests COMMAND whatson_detail_panel_viewmodel_tests)
list(APPEND WHATSON_TEST_TARGETS whatson_detail_panel_viewmodel_tests)

add_executable(whatson_navigationbar_viewmodel_tests
        app/test_navigation_mode_viewmodel.cpp
        ${CMAKE_SOURCE_DIR}/src/app/viewmodel/navigationbar/NavigationModeState.cpp
        ${CMAKE_SOURCE_DIR}/src/app/viewmodel/navigationbar/NavigationModeSectionViewModel.cpp
        ${CMAKE_SOURCE_DIR}/src/app/viewmodel/navigationbar/NavigationModeViewModel.cpp
)
set_target_properties(whatson_navigationbar_viewmodel_tests PROPERTIES AUTOMOC ON)
target_include_directories(whatson_navigationbar_viewmodel_tests
        PRIVATE
        ${CMAKE_SOURCE_DIR}/src/app
)
target_link_libraries(whatson_navigationbar_viewmodel_tests
        PRIVATE
        Qt6::Core
        Qt6::Test
)
add_test(NAME whatson_navigationbar_viewmodel_tests COMMAND whatson_navigationbar_viewmodel_tests)
list(APPEND WHATSON_TEST_TARGETS whatson_navigationbar_viewmodel_tests)

add_executable(whatson_editor_view_mode_viewmodel_tests
        app/test_editor_view_mode_viewmodel.cpp
        ${CMAKE_SOURCE_DIR}/src/app/viewmodel/navigationbar/EditorViewState.cpp
        ${CMAKE_SOURCE_DIR}/src/app/viewmodel/navigationbar/EditorViewSectionViewModel.cpp
        ${CMAKE_SOURCE_DIR}/src/app/viewmodel/navigationbar/EditorViewModeViewModel.cpp
)
set_target_properties(whatson_editor_view_mode_viewmodel_tests PROPERTIES AUTOMOC ON)
target_include_directories(whatson_editor_view_mode_viewmodel_tests
        PRIVATE
        ${CMAKE_SOURCE_DIR}/src/app
)
target_link_libraries(whatson_editor_view_mode_viewmodel_tests
        PRIVATE
        Qt6::Core
        Qt6::Test
)
add_test(NAME whatson_editor_view_mode_viewmodel_tests COMMAND whatson_editor_view_mode_viewmodel_tests)
list(APPEND WHATSON_TEST_TARGETS whatson_editor_view_mode_viewmodel_tests)

add_executable(whatson_note_header_tests
        app/test_whatson_note_header_parser.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/note/WhatSonNoteCreator.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/note/WhatSonNoteHeaderCreator.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/note/WhatSonNoteBodyCreator.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/note/WhatSonNoteLinkManagerCreator.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/note/WhatSonNoteAttachManagerCreator.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/note/WhatSonNoteHeaderStore.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/note/WhatSonNoteHeaderParser.cpp
)
set_target_properties(whatson_note_header_tests PROPERTIES AUTOMOC ON)
target_include_directories(whatson_note_header_tests
        PRIVATE
        ${CMAKE_SOURCE_DIR}/src/app/file
        ${CMAKE_SOURCE_DIR}/src/app/file/note
)
target_link_libraries(whatson_note_header_tests
        PRIVATE
        Qt6::Core
        Qt6::Test
)
add_test(NAME whatson_note_header_tests COMMAND whatson_note_header_tests)
list(APPEND WHATSON_TEST_TARGETS whatson_note_header_tests)

add_executable(whatson_hierarchy_io_tests
        app/test_whatson_hierarchy_io.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/library/WhatSonLibraryHierarchyStore.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/library/WhatSonLibraryHierarchyParser.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/library/WhatSonLibraryHierarchyCreator.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/projects/WhatSonProjectsHierarchyStore.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/projects/WhatSonProjectsHierarchyParser.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/projects/WhatSonProjectsHierarchyCreator.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/bookmarks/WhatSonBookmarksHierarchyStore.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/bookmarks/WhatSonBookmarksHierarchyParser.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/bookmarks/WhatSonBookmarksHierarchyCreator.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/resources/WhatSonResourcesHierarchyStore.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/resources/WhatSonResourcesHierarchyParser.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/resources/WhatSonResourcesHierarchyCreator.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/event/WhatSonEventHierarchyStore.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/event/WhatSonEventHierarchyParser.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/event/WhatSonEventHierarchyCreator.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/preset/WhatSonPresetHierarchyStore.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/preset/WhatSonPresetHierarchyParser.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/preset/WhatSonPresetHierarchyCreator.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/progress/WhatSonProgressHierarchyStore.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/progress/WhatSonProgressHierarchyParser.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/progress/WhatSonProgressHierarchyCreator.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/tags/WhatSonTagsHierarchyStore.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/tags/WhatSonTagsHierarchyParser.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/tags/WhatSonTagsHierarchyCreator.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/tags/WhatSonTagsJsonParser.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/tags/WhatSonTagsDepthFlattener.cpp
)
set_target_properties(whatson_hierarchy_io_tests PROPERTIES AUTOMOC ON)
target_include_directories(whatson_hierarchy_io_tests
        PRIVATE
        ${CMAKE_SOURCE_DIR}/src/app/file
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/library
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/projects
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/bookmarks
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/resources
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/event
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/preset
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/progress
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/tags
)
target_link_libraries(whatson_hierarchy_io_tests
        PRIVATE
        Qt6::Core
        Qt6::Test
)
add_test(NAME whatson_hierarchy_io_tests COMMAND whatson_hierarchy_io_tests)
list(APPEND WHATSON_TEST_TARGETS whatson_hierarchy_io_tests)

add_executable(whatson_io_runtime_tests
        app/test_whatson_io_runtime.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/IO/WhatSonIoEventListener.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/IO/WhatSonSystemIoGateway.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/IO/WhatSonIoRuntimeController.cpp
)
set_target_properties(whatson_io_runtime_tests PROPERTIES AUTOMOC ON)
target_include_directories(whatson_io_runtime_tests
        PRIVATE
        ${CMAKE_SOURCE_DIR}/src/app
        ${CMAKE_SOURCE_DIR}/src/app/file
        ${CMAKE_SOURCE_DIR}/src/app/file/IO
)
target_link_libraries(whatson_io_runtime_tests
        PRIVATE
        Qt6::Core
        Qt6::Test
)
add_test(NAME whatson_io_runtime_tests COMMAND whatson_io_runtime_tests)
list(APPEND WHATSON_TEST_TARGETS whatson_io_runtime_tests)

add_executable(whatson_permission_bridge_source_guard_tests
        app/test_permission_bridge_source_guard.cpp
)
set_target_properties(whatson_permission_bridge_source_guard_tests PROPERTIES AUTOMOC ON)
target_include_directories(whatson_permission_bridge_source_guard_tests
        PRIVATE
        ${CMAKE_SOURCE_DIR}/src/app
)
target_link_libraries(whatson_permission_bridge_source_guard_tests
        PRIVATE
        Qt6::Core
        Qt6::Test
)
add_test(NAME whatson_permission_bridge_source_guard_tests COMMAND whatson_permission_bridge_source_guard_tests)
list(APPEND WHATSON_TEST_TARGETS whatson_permission_bridge_source_guard_tests)

if (WHATSON_TEST_TARGETS)
    add_custom_target(whatson_tests DEPENDS ${WHATSON_TEST_TARGETS})
endif ()
