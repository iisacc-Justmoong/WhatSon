if (NOT DEFINED QMLFORMAT_EXECUTABLE OR QMLFORMAT_EXECUTABLE STREQUAL "")
    message(FATAL_ERROR "QMLFORMAT_EXECUTABLE is required.")
endif ()

if (NOT DEFINED QML_FILELIST OR QML_FILELIST STREQUAL "")
    message(FATAL_ERROR "QML_FILELIST is required.")
endif ()

if (NOT EXISTS "${QML_FILELIST}")
    message(FATAL_ERROR "QML file list does not exist: ${QML_FILELIST}")
endif ()

file(STRINGS "${QML_FILELIST}" WHATSON_QML_FILES)

set(WHATSON_QMLFORMAT_MISMATCHES)
foreach (WHATSON_QML_FILE IN LISTS WHATSON_QML_FILES)
    if (WHATSON_QML_FILE STREQUAL "")
        continue()
    endif ()

    execute_process(
            COMMAND "${QMLFORMAT_EXECUTABLE}" "${WHATSON_QML_FILE}"
            RESULT_VARIABLE WHATSON_QMLFORMAT_RESULT
            OUTPUT_VARIABLE WHATSON_QMLFORMAT_OUTPUT
            ERROR_VARIABLE WHATSON_QMLFORMAT_ERROR
    )

    if (NOT WHATSON_QMLFORMAT_RESULT EQUAL 0)
        message(FATAL_ERROR
                "qmlformat failed for ${WHATSON_QML_FILE}\n${WHATSON_QMLFORMAT_ERROR}")
    endif ()

    file(READ "${WHATSON_QML_FILE}" WHATSON_QML_CURRENT_CONTENTS)
    if (NOT WHATSON_QML_CURRENT_CONTENTS STREQUAL WHATSON_QMLFORMAT_OUTPUT)
        list(APPEND WHATSON_QMLFORMAT_MISMATCHES "${WHATSON_QML_FILE}")
    endif ()
endforeach ()

if (WHATSON_QMLFORMAT_MISMATCHES)
    list(JOIN WHATSON_QMLFORMAT_MISMATCHES "\n" WHATSON_QMLFORMAT_MISMATCH_TEXT)
    message(FATAL_ERROR
            "qmlformat check failed for the following files:\n${WHATSON_QMLFORMAT_MISMATCH_TEXT}\n"
            "Run `cmake --build build --target whatson_qmlformat_fix` to rewrite them in-place.")
endif ()

message(STATUS "qmlformat check passed for ${QML_FILELIST}")
