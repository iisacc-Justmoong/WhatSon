set(_whatson_platform_patterns
        "*.cpp"
        "*.h"
        "*.hpp")

if (APPLE)
    list(APPEND _whatson_platform_patterns "*.mm")
endif ()

whatson_app_register_directory_sources(
        "${CMAKE_CURRENT_SOURCE_DIR}"
        RECURSE
        PATTERNS ${_whatson_platform_patterns})
whatson_app_register_directory_include_directories("${CMAKE_CURRENT_SOURCE_DIR}")
