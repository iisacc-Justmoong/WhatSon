add_custom_target(whatson_clean_build_extras
        COMMAND "${CMAKE_COMMAND}"
        "-DWHATSON_BUILD_DIR=${CMAKE_BINARY_DIR}"
        -P "${CMAKE_SOURCE_DIR}/cmake/CleanWhatSonBuildExtras.cmake"
        COMMENT "Remove stale WhatSon diagnostic files from the build tree"
)

if (BUILD_TESTING)
    if (TARGET whatson_cpp_regression AND TARGET whatson_cpp_regression_tests)
        add_dependencies(whatson_cpp_regression whatson_cpp_regression_tests)
    endif ()

    set(WHATSON_REGRESSION_BUILD_TARGETS)
    if (TARGET whatson_build_all)
        list(APPEND WHATSON_REGRESSION_BUILD_TARGETS whatson_build_all)
    endif ()
    if (TARGET whatson_cpp_regression_tests)
        list(APPEND WHATSON_REGRESSION_BUILD_TARGETS whatson_cpp_regression_tests)
    endif ()

    if (WHATSON_REGRESSION_BUILD_TARGETS)
        add_custom_target(whatson_build_regression
                DEPENDS ${WHATSON_REGRESSION_BUILD_TARGETS}
                COMMENT "Build maintained WhatSon product and regression binaries"
        )
    endif ()

    if (TARGET whatson_build_regression AND TARGET whatson_cpp_regression)
        add_custom_target(whatson_regression
                COMMAND "${CMAKE_COMMAND}"
                "-DWHATSON_BUILD_DIR=${CMAKE_BINARY_DIR}"
                -P "${CMAKE_SOURCE_DIR}/cmake/CleanWhatSonBuildExtras.cmake"
                COMMAND "${CMAKE_CTEST_COMMAND}" --output-on-failure -L cpp_regression
                COMMAND "${CMAKE_COMMAND}"
                "-DWHATSON_BUILD_DIR=${CMAKE_BINARY_DIR}"
                -P "${CMAKE_SOURCE_DIR}/cmake/CleanWhatSonBuildExtras.cmake"
                WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
                USES_TERMINAL
                COMMENT "Build WhatSon and run maintained regression gates"
        )
        add_dependencies(whatson_regression whatson_build_regression)
        add_dependencies(whatson_regression whatson_clean_build_extras)
    endif ()
endif ()
