find_package(Qt6 6.5 REQUIRED COMPONENTS Core Test)

set(WHATSON_TEST_TARGETS)

add_executable(whatson_hub_creator_tests
        app/test_whatson_workspace_hub_creator.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hub/WhatSonHubCreator.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hub/WhatSonWorkspaceHubCreator.cpp
)
set_target_properties(whatson_hub_creator_tests PROPERTIES AUTOMOC ON)
target_include_directories(whatson_hub_creator_tests
        PRIVATE
        ${CMAKE_SOURCE_DIR}/src/app/file/hub
)
target_link_libraries(whatson_hub_creator_tests
        PRIVATE
        Qt6::Core
        Qt6::Test
)
add_test(NAME whatson_hub_creator_tests COMMAND whatson_hub_creator_tests)
list(APPEND WHATSON_TEST_TARGETS whatson_hub_creator_tests)

add_executable(whatson_sidebar_store_tests
        app/test_sidebar_hierarchy_store.cpp
        ${CMAKE_SOURCE_DIR}/src/app/viewmodel/sidebar/HierarchyItemListModel.cpp
        ${CMAKE_SOURCE_DIR}/src/app/viewmodel/sidebar/HierarchySectionModel.cpp
        ${CMAKE_SOURCE_DIR}/src/app/viewmodel/sidebar/SidebarHierarchyStore.cpp
)
set_target_properties(whatson_sidebar_store_tests PROPERTIES AUTOMOC ON)
target_include_directories(whatson_sidebar_store_tests
        PRIVATE
        ${CMAKE_SOURCE_DIR}/src/app/viewmodel
        ${CMAKE_SOURCE_DIR}/src/app/viewmodel/sidebar
)
target_link_libraries(whatson_sidebar_store_tests
        PRIVATE
        Qt6::Core
        Qt6::Test
)
add_test(NAME whatson_sidebar_store_tests COMMAND whatson_sidebar_store_tests)
list(APPEND WHATSON_TEST_TARGETS whatson_sidebar_store_tests)

add_executable(whatson_library_hierarchy_tests
        app/test_library_hierarchy_view_model.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/library/LibraryHierarchyModel.cpp
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/library/LibraryHierarchyViewModel.cpp
)
set_target_properties(whatson_library_hierarchy_tests PROPERTIES AUTOMOC ON)
target_include_directories(whatson_library_hierarchy_tests
        PRIVATE
        ${CMAKE_SOURCE_DIR}/src/app/file
        ${CMAKE_SOURCE_DIR}/src/app/file/hierarchy/library
)
target_link_libraries(whatson_library_hierarchy_tests
        PRIVATE
        Qt6::Core
        Qt6::Test
)
add_test(NAME whatson_library_hierarchy_tests COMMAND whatson_library_hierarchy_tests)
list(APPEND WHATSON_TEST_TARGETS whatson_library_hierarchy_tests)

if (WHATSON_TEST_TARGETS)
    add_custom_target(whatson_tests DEPENDS ${WHATSON_TEST_TARGETS})
endif ()
