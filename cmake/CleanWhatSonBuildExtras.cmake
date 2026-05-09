if (NOT DEFINED WHATSON_BUILD_DIR OR WHATSON_BUILD_DIR STREQUAL "")
    message(FATAL_ERROR "WHATSON_BUILD_DIR is required.")
endif ()

file(TO_CMAKE_PATH "${WHATSON_BUILD_DIR}" _whatson_build_dir)
if (NOT IS_DIRECTORY "${_whatson_build_dir}")
    return()
endif ()

set(_whatson_build_extra_paths
        "${_whatson_build_dir}/cmake-trace-qt6core.log"
        "${_whatson_build_dir}/whatson_blank_window_diagnostic.log"
        "${_whatson_build_dir}/whatson_lvrs_bootstrap_smoke.log"
        "${_whatson_build_dir}/whatson_qmllint.latest.log"
        "${_whatson_build_dir}/whatson_qmllint_diagnostic.log"
        "${_whatson_build_dir}/whatson_cpp_files.txt"
        "${_whatson_build_dir}/whatson_qml_files.txt"
)

file(GLOB _whatson_root_diagnostic_images
        "${_whatson_build_dir}/whatson-*.png"
)
file(GLOB _whatson_root_body_backups
        "${_whatson_build_dir}/*.wsnbody.backup.xml"
        "${_whatson_build_dir}/*.wsnbody.pre-*-backup.xml"
)
file(GLOB_RECURSE _whatson_finder_metadata
        "${_whatson_build_dir}/.DS_Store"
        "${_whatson_build_dir}/*/.DS_Store"
)

list(APPEND _whatson_build_extra_paths
        ${_whatson_root_diagnostic_images}
        ${_whatson_root_body_backups}
        ${_whatson_finder_metadata}
)

foreach (_whatson_build_extra_path IN LISTS _whatson_build_extra_paths)
    if (EXISTS "${_whatson_build_extra_path}")
        file(REMOVE "${_whatson_build_extra_path}")
    endif ()
endforeach ()
