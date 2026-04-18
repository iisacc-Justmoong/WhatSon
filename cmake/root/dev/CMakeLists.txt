function(_whatson_add_missing_tool_target target_name tool_name install_hint)
    add_custom_target(${target_name}
            COMMAND "${CMAKE_COMMAND}" -E echo "${tool_name} was not found. ${install_hint}"
            COMMAND "${CMAKE_COMMAND}" -E false
            USES_TERMINAL
            COMMENT "Missing developer tool: ${tool_name}"
    )
endfunction()

if (WHATSON_ENABLE_DEV_TOOLING)
    if (APPLE)
        set(_whatson_host_qml_platform "macos")
    elseif (WIN32)
        set(_whatson_host_qml_platform "windows")
    else ()
        set(_whatson_host_qml_platform "linux")
    endif ()

    set(_whatson_qt_bin_hints)
    if (QT_ROOT_PATH AND EXISTS "${QT_ROOT_PATH}/bin")
        list(APPEND _whatson_qt_bin_hints "${QT_ROOT_PATH}/bin")
    endif ()

    find_program(WHATSON_QMLLINT_EXECUTABLE NAMES qmllint HINTS ${_whatson_qt_bin_hints})
    find_program(WHATSON_QMLFORMAT_EXECUTABLE NAMES qmlformat HINTS ${_whatson_qt_bin_hints})
    find_program(WHATSON_CLANG_TIDY_EXECUTABLE NAMES clang-tidy clang-tidy-18 clang-tidy-17 clang-tidy-16)

    message(STATUS "  WHATSON_QMLLINT_EXECUTABLE: ${WHATSON_QMLLINT_EXECUTABLE}")
    message(STATUS "  WHATSON_QMLFORMAT_EXECUTABLE: ${WHATSON_QMLFORMAT_EXECUTABLE}")
    message(STATUS "  WHATSON_CLANG_TIDY_EXECUTABLE: ${WHATSON_CLANG_TIDY_EXECUTABLE}")

    file(GLOB_RECURSE WHATSON_DEV_QML_FILES
            CONFIGURE_DEPENDS
            RELATIVE "${CMAKE_SOURCE_DIR}"
            "${CMAKE_SOURCE_DIR}/src/app/qml/*.qml"
    )

    file(GLOB_RECURSE WHATSON_DEV_CPP_FILES
            CONFIGURE_DEPENDS
            RELATIVE "${CMAKE_SOURCE_DIR}"
            "${CMAKE_SOURCE_DIR}/src/app/*.cpp"
            "${CMAKE_SOURCE_DIR}/src/app/*.cc"
            "${CMAKE_SOURCE_DIR}/src/app/*.cxx"
    )

    if (WHATSON_BUILD_DAEMON)
        file(GLOB_RECURSE WHATSON_DEV_DAEMON_CPP_FILES
                CONFIGURE_DEPENDS
                RELATIVE "${CMAKE_SOURCE_DIR}"
                "${CMAKE_SOURCE_DIR}/src/daemon/*.cpp"
                "${CMAKE_SOURCE_DIR}/src/daemon/*.cc"
                "${CMAKE_SOURCE_DIR}/src/daemon/*.cxx"
        )
        list(APPEND WHATSON_DEV_CPP_FILES ${WHATSON_DEV_DAEMON_CPP_FILES})
    endif ()

    list(REMOVE_DUPLICATES WHATSON_DEV_QML_FILES)
    list(REMOVE_DUPLICATES WHATSON_DEV_CPP_FILES)
    list(SORT WHATSON_DEV_QML_FILES)
    list(SORT WHATSON_DEV_CPP_FILES)

    set(WHATSON_DEV_QML_FILES_ABS ${WHATSON_DEV_QML_FILES})
    list(TRANSFORM WHATSON_DEV_QML_FILES_ABS PREPEND "${CMAKE_SOURCE_DIR}/")

    set(WHATSON_DEV_CPP_FILES_ABS ${WHATSON_DEV_CPP_FILES})
    list(TRANSFORM WHATSON_DEV_CPP_FILES_ABS PREPEND "${CMAKE_SOURCE_DIR}/")

    set(WHATSON_DEV_QML_FILELIST "${CMAKE_BINARY_DIR}/whatson_qml_files.txt")
    string(REPLACE ";" "\n" _whatson_dev_qml_filelist_text "${WHATSON_DEV_QML_FILES_ABS}")
    file(GENERATE OUTPUT "${WHATSON_DEV_QML_FILELIST}" CONTENT "${_whatson_dev_qml_filelist_text}\n")

    set(WHATSON_DEV_CPP_FILELIST "${CMAKE_BINARY_DIR}/whatson_cpp_files.txt")
    string(REPLACE ";" "\n" _whatson_dev_cpp_filelist_text "${WHATSON_DEV_CPP_FILES_ABS}")
    file(GENERATE OUTPUT "${WHATSON_DEV_CPP_FILELIST}" CONTENT "${_whatson_dev_cpp_filelist_text}\n")

    set(_whatson_qml_import_dirs)
    if (LVRS_PREFIX AND EXISTS "${LVRS_PREFIX}/platforms/${_whatson_host_qml_platform}/lib/qt6/qml")
        list(APPEND _whatson_qml_import_dirs "${LVRS_PREFIX}/platforms/${_whatson_host_qml_platform}/lib/qt6/qml")
    endif ()
    if (QT_ROOT_PATH AND EXISTS "${QT_ROOT_PATH}/qml")
        list(APPEND _whatson_qml_import_dirs "${QT_ROOT_PATH}/qml")
    endif ()
    if (EXISTS "${CMAKE_BINARY_DIR}/src/app")
        list(APPEND _whatson_qml_import_dirs "${CMAKE_BINARY_DIR}/src/app")
    endif ()
    list(REMOVE_DUPLICATES _whatson_qml_import_dirs)

    if (WHATSON_QMLLINT_EXECUTABLE)
        set(_whatson_qmllint_args "--max-warnings" "-1")
        foreach (_whatson_qml_import_dir IN LISTS _whatson_qml_import_dirs)
            list(APPEND _whatson_qmllint_args "-I" "${_whatson_qml_import_dir}")
        endforeach ()
        if (EXISTS "${CMAKE_BINARY_DIR}/src/app/WhatSon/App/qmldir")
            list(APPEND _whatson_qmllint_args "-i" "${CMAKE_BINARY_DIR}/src/app/WhatSon/App/qmldir")
        endif ()
        list(APPEND _whatson_qmllint_args ${WHATSON_DEV_QML_FILES_ABS})

        add_custom_target(whatson_qmllint
                COMMAND "${WHATSON_QMLLINT_EXECUTABLE}" ${_whatson_qmllint_args}
                USES_TERMINAL
                COMMENT "Run qmllint on WhatSon QML sources"
        )
    else ()
        _whatson_add_missing_tool_target(
                whatson_qmllint
                "qmllint"
                "Install Qt QML tools or set QT_ROOT_PATH to a Qt prefix that contains qmllint.")
    endif ()

    if (WHATSON_QMLFORMAT_EXECUTABLE)
        add_custom_target(whatson_qmlformat_check
                COMMAND "${CMAKE_COMMAND}"
                "-DQMLFORMAT_EXECUTABLE=${WHATSON_QMLFORMAT_EXECUTABLE}"
                "-DQML_FILELIST=${WHATSON_DEV_QML_FILELIST}"
                -P "${CMAKE_SOURCE_DIR}/cmake/RunWhatSonQmlFormatCheck.cmake"
                USES_TERMINAL
                COMMENT "Check QML formatting with qmlformat"
        )

        add_custom_target(whatson_qmlformat_fix
                COMMAND "${WHATSON_QMLFORMAT_EXECUTABLE}" --inplace --files "${WHATSON_DEV_QML_FILELIST}"
                USES_TERMINAL
                COMMENT "Rewrite WhatSon QML sources in-place with qmlformat"
        )
    else ()
        _whatson_add_missing_tool_target(
                whatson_qmlformat_check
                "qmlformat"
                "Install Qt QML tools or set QT_ROOT_PATH to a Qt prefix that contains qmlformat.")
        _whatson_add_missing_tool_target(
                whatson_qmlformat_fix
                "qmlformat"
                "Install Qt QML tools or set QT_ROOT_PATH to a Qt prefix that contains qmlformat.")
    endif ()

    if (WHATSON_CLANG_TIDY_EXECUTABLE)
        add_custom_target(whatson_clang_tidy
                COMMAND "${CMAKE_COMMAND}"
                "-DCLANG_TIDY_EXECUTABLE=${WHATSON_CLANG_TIDY_EXECUTABLE}"
                "-DCPP_FILELIST=${WHATSON_DEV_CPP_FILELIST}"
                "-DBUILD_DIR=${CMAKE_BINARY_DIR}"
                -P "${CMAKE_SOURCE_DIR}/cmake/RunWhatSonClangTidy.cmake"
                USES_TERMINAL
                COMMENT "Run clang-tidy on WhatSon C++ sources"
        )
    else ()
        _whatson_add_missing_tool_target(
                whatson_clang_tidy
                "clang-tidy"
                "Install clang-tidy and re-run CMake configure so the target can bind to the executable.")
    endif ()

    add_custom_target(whatson_dev_checks
            DEPENDS
            whatson_qmllint
            whatson_qmlformat_check
            COMMENT "Run the default WhatSon developer lint checks"
    )
    if (WHATSON_CLANG_TIDY_EXECUTABLE)
        add_dependencies(whatson_dev_checks whatson_clang_tidy)
    endif ()
endif ()
