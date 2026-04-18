set(_whatson_permissions_patterns
        "*.cpp"
        "*.h"
        "*.hpp")
set(_whatson_permissions_exclude_regex)

if (APPLE)
    list(APPEND _whatson_permissions_patterns "*.mm")
    list(APPEND _whatson_permissions_exclude_regex "ApplePermissionBridge_stub\\.cpp$")
endif ()

whatson_app_register_directory_sources(
        "${CMAKE_CURRENT_SOURCE_DIR}"
        RECURSE
        PATTERNS ${_whatson_permissions_patterns}
        EXCLUDE_REGEX ${_whatson_permissions_exclude_regex})
whatson_app_register_directory_include_directories("${CMAKE_CURRENT_SOURCE_DIR}")
