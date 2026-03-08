if (NOT DEFINED CLANG_TIDY_EXECUTABLE OR CLANG_TIDY_EXECUTABLE STREQUAL "")
    message(FATAL_ERROR "CLANG_TIDY_EXECUTABLE is required.")
endif ()

if (NOT EXISTS "${CLANG_TIDY_EXECUTABLE}")
    message(FATAL_ERROR
            "clang-tidy executable was not found: ${CLANG_TIDY_EXECUTABLE}\n"
            "Install clang-tidy and re-run CMake configure.")
endif ()

if (NOT DEFINED CPP_FILELIST OR CPP_FILELIST STREQUAL "")
    message(FATAL_ERROR "CPP_FILELIST is required.")
endif ()

if (NOT EXISTS "${CPP_FILELIST}")
    message(FATAL_ERROR "C++ file list does not exist: ${CPP_FILELIST}")
endif ()

if (NOT DEFINED BUILD_DIR OR BUILD_DIR STREQUAL "")
    message(FATAL_ERROR "BUILD_DIR is required.")
endif ()

if (NOT EXISTS "${BUILD_DIR}/compile_commands.json")
    message(FATAL_ERROR
            "compile_commands.json was not found in ${BUILD_DIR}\n"
            "Run `cmake -S . -B ${BUILD_DIR}` first.")
endif ()

file(STRINGS "${CPP_FILELIST}" WHATSON_CPP_FILES)

set(WHATSON_CLANG_TIDY_FAILURES)
set(WHATSON_CLANG_TIDY_LOG "")
foreach (WHATSON_CPP_FILE IN LISTS WHATSON_CPP_FILES)
    if (WHATSON_CPP_FILE STREQUAL "")
        continue()
    endif ()

    execute_process(
            COMMAND "${CLANG_TIDY_EXECUTABLE}" -p "${BUILD_DIR}" -quiet "${WHATSON_CPP_FILE}"
            RESULT_VARIABLE WHATSON_CLANG_TIDY_RESULT
            OUTPUT_VARIABLE WHATSON_CLANG_TIDY_OUTPUT
            ERROR_VARIABLE WHATSON_CLANG_TIDY_ERROR
    )

    if (NOT WHATSON_CLANG_TIDY_RESULT EQUAL 0)
        list(APPEND WHATSON_CLANG_TIDY_FAILURES "${WHATSON_CPP_FILE}")
        string(APPEND WHATSON_CLANG_TIDY_LOG
                "\n[${WHATSON_CPP_FILE}]\n${WHATSON_CLANG_TIDY_OUTPUT}${WHATSON_CLANG_TIDY_ERROR}\n")
    endif ()
endforeach ()

if (WHATSON_CLANG_TIDY_FAILURES)
    list(JOIN WHATSON_CLANG_TIDY_FAILURES "\n" WHATSON_CLANG_TIDY_FAILURE_TEXT)
    message(FATAL_ERROR
            "clang-tidy reported issues in the following files:\n${WHATSON_CLANG_TIDY_FAILURE_TEXT}\n"
            "${WHATSON_CLANG_TIDY_LOG}")
endif ()

message(STATUS "clang-tidy passed for ${CPP_FILELIST}")
