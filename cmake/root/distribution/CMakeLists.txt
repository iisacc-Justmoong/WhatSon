set(WHATSON_DIST_DIR "${CMAKE_BINARY_DIR}/dist")
set(_whatson_native_desktop_install_tree FALSE)
if (NOT CMAKE_CROSSCOMPILING)
    if (WIN32)
        set(_whatson_native_desktop_install_tree TRUE)
    elseif (APPLE AND NOT CMAKE_SYSTEM_NAME STREQUAL "iOS")
        set(_whatson_native_desktop_install_tree TRUE)
    elseif (UNIX AND NOT APPLE)
        set(_whatson_native_desktop_install_tree TRUE)
    endif ()
endif ()

if (WHATSON_BUILD_TARGETS)
    if (_whatson_native_desktop_install_tree)
        set(_whatson_install_dist_command
                "${CMAKE_COMMAND}"
                --install "${CMAKE_BINARY_DIR}"
                --prefix "${WHATSON_DIST_DIR}")
        if (CMAKE_CONFIGURATION_TYPES)
            list(APPEND _whatson_install_dist_command --config "$<CONFIG>")
        endif ()

        add_custom_target(whatson_export_binaries
                COMMAND "${CMAKE_COMMAND}" -E rm -rf "${WHATSON_DIST_DIR}"
                COMMAND "${CMAKE_COMMAND}" -E make_directory "${WHATSON_DIST_DIR}"
                COMMAND ${_whatson_install_dist_command}
                COMMENT "Export self-contained host install tree to ${WHATSON_DIST_DIR}"
        )
    else ()
        add_custom_target(whatson_export_binaries
                COMMAND "${CMAKE_COMMAND}" -E rm -rf "${WHATSON_DIST_DIR}"
                COMMAND "${CMAKE_COMMAND}" -E make_directory "${WHATSON_DIST_DIR}"
                COMMENT "Export host binaries to ${WHATSON_DIST_DIR}"
        )

        if (TARGET WhatSon)
            if (APPLE)
                add_custom_command(TARGET whatson_export_binaries POST_BUILD
                        COMMAND "${CMAKE_COMMAND}" -E copy_directory
                        "$<TARGET_BUNDLE_DIR:WhatSon>"
                        "${WHATSON_DIST_DIR}/WhatSon.app"
                )
            else ()
                add_custom_command(TARGET whatson_export_binaries POST_BUILD
                        COMMAND "${CMAKE_COMMAND}" -E copy
                        "$<TARGET_FILE:WhatSon>"
                        "${WHATSON_DIST_DIR}/$<TARGET_FILE_NAME:WhatSon>"
                )
            endif ()
        endif ()

        if (TARGET WhatSon_daemon)
            add_custom_command(TARGET whatson_export_binaries POST_BUILD
                    COMMAND "${CMAKE_COMMAND}" -E copy
                    "$<TARGET_FILE:WhatSon_daemon>"
                    "${WHATSON_DIST_DIR}/$<TARGET_FILE_NAME:WhatSon_daemon>"
            )
        endif ()

        if (TARGET whatson_cli AND DEFINED WHATSON_CLI_BINARY_PATH AND NOT WHATSON_CLI_BINARY_PATH STREQUAL "")
            add_custom_command(TARGET whatson_export_binaries POST_BUILD
                    COMMAND "${CMAKE_COMMAND}" -E copy
                    "${WHATSON_CLI_BINARY_PATH}"
                    "${WHATSON_DIST_DIR}/whatson${CMAKE_EXECUTABLE_SUFFIX}"
            )
        endif ()
    endif ()

    add_dependencies(whatson_export_binaries whatson_build_all)
endif ()

include(GNUInstallDirs)
set(WHATSON_LINUX_DESKTOP_FILE "")
if (UNIX AND NOT APPLE AND EXISTS "${WHATSON_LINUX_DIR}/WhatSon.desktop.in")
    set(WHATSON_LINUX_DESKTOP_FILE "${CMAKE_CURRENT_BINARY_DIR}/WhatSon.desktop")
    configure_file("${WHATSON_LINUX_DIR}/WhatSon.desktop.in" "${WHATSON_LINUX_DESKTOP_FILE}" @ONLY)
endif ()

if (TARGET WhatSon)
    install(TARGETS WhatSon
            BUNDLE DESTINATION .
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif ()
if (TARGET WhatSon_daemon)
    if (APPLE)
        install(TARGETS WhatSon_daemon
                BUNDLE DESTINATION .
                LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
                RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        )
    else ()
        install(TARGETS WhatSon_daemon
                LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
                RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        )
    endif ()
endif ()

if (TARGET whatson_cli AND DEFINED WHATSON_CLI_BINARY_PATH AND NOT WHATSON_CLI_BINARY_PATH STREQUAL "")
    install(PROGRAMS "${WHATSON_CLI_BINARY_PATH}" DESTINATION ${CMAKE_INSTALL_BINDIR})
endif ()

if (TARGET WhatSon AND UNIX AND NOT APPLE)
    if (NOT WHATSON_LINUX_DESKTOP_FILE STREQUAL "")
        install(FILES "${WHATSON_LINUX_DESKTOP_FILE}" DESTINATION "${CMAKE_INSTALL_DATADIR}/applications")
    endif ()

    if (EXISTS "${WHATSON_APP_ICON_PNG}")
        install(FILES "${WHATSON_APP_ICON_PNG}"
                DESTINATION "${CMAKE_INSTALL_DATADIR}/icons/hicolor/512x512/apps"
                RENAME "whatson.png")
        install(FILES "${WHATSON_APP_ICON_PNG}"
                DESTINATION "${CMAKE_INSTALL_DATADIR}/pixmaps"
                RENAME "whatson.png")
    endif ()
endif ()

if (TARGET WhatSon
        AND _whatson_native_desktop_install_tree
        AND COMMAND qt_generate_deploy_qml_app_script)
    qt_generate_deploy_qml_app_script(
            TARGET WhatSon
            OUTPUT_SCRIPT WHATSON_QT_DEPLOY_APP_SCRIPT
            NO_UNSUPPORTED_PLATFORM_ERROR
    )
    install(SCRIPT "${WHATSON_QT_DEPLOY_APP_SCRIPT}")
endif ()

set(_whatson_native_desktop_runtime_deploy FALSE)
if (NOT CMAKE_CROSSCOMPILING)
    if (WIN32)
        set(_whatson_native_desktop_runtime_deploy TRUE)
    elseif (UNIX AND NOT APPLE)
        set(_whatson_native_desktop_runtime_deploy TRUE)
    endif ()
endif ()

if (TARGET WhatSon_daemon
        AND _whatson_native_desktop_runtime_deploy
        AND COMMAND qt_generate_deploy_script)
    set(_whatson_daemon_qt_conf_arg "")
    if (WIN32)
        set(_whatson_daemon_qt_conf_arg "    GENERATE_QT_CONF\n")
    endif ()

    qt_generate_deploy_script(
            TARGET WhatSon_daemon
            OUTPUT_SCRIPT WHATSON_QT_DEPLOY_DAEMON_SCRIPT
            CONTENT "
qt_deploy_runtime_dependencies(
    EXECUTABLE \$<TARGET_FILE:WhatSon_daemon>
${_whatson_daemon_qt_conf_arg}\
    NO_TRANSLATIONS
)
")
    install(SCRIPT "${WHATSON_QT_DEPLOY_DAEMON_SCRIPT}")
endif ()

set(CPACK_PACKAGE_NAME "WhatSon")
set(CPACK_PACKAGE_VENDOR "LVRS")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
if (APPLE)
    set(CPACK_GENERATOR "DragNDrop;ZIP")
elseif (WIN32)
    set(CPACK_GENERATOR "ZIP")
elseif (UNIX)
    set(CPACK_GENERATOR "TGZ;ZIP")
endif ()
include(CPack)

if (CMAKE_CPACK_COMMAND)
    add_custom_target(whatson_package
            COMMAND "${CMAKE_CPACK_COMMAND}" --config "${CMAKE_BINARY_DIR}/CPackConfig.cmake"
            USES_TERMINAL
            COMMENT "Generate distributable package for current platform"
    )
    if (TARGET whatson_build_all)
        add_dependencies(whatson_package whatson_build_all)
    endif ()
endif ()

if (TARGET whatson_package
        AND TARGET WhatSon
        AND NOT CMAKE_CROSSCOMPILING
        AND NOT WHATSON_IS_TRIAL_BUILD
        AND WHATSON_ENABLE_TRIAL_BUILD_MIRROR)
    set(_whatson_trial_cache_script "${CMAKE_BINARY_DIR}/CMakeFiles/whatson_trial_cache_init.cmake")
    set(_whatson_trial_cache_lines "")
    if (DEFINED CMAKE_PREFIX_PATH AND NOT CMAKE_PREFIX_PATH STREQUAL "")
        string(APPEND _whatson_trial_cache_lines
                "set(CMAKE_PREFIX_PATH [==[${CMAKE_PREFIX_PATH}]==] CACHE STRING \"Forwarded host prefix path list for the trial mirror.\" FORCE)\n")
    endif ()
    if (APPLE
            AND DEFINED WHATSON_TRIAL_BUILD_OSX_ARCHITECTURES
            AND NOT WHATSON_TRIAL_BUILD_OSX_ARCHITECTURES STREQUAL "")
        string(APPEND _whatson_trial_cache_lines
                "set(CMAKE_OSX_ARCHITECTURES [==[${WHATSON_TRIAL_BUILD_OSX_ARCHITECTURES}]==] CACHE STRING \"Forwarded macOS architecture list for the trial mirror.\" FORCE)\n")
    endif ()
    if (NOT _whatson_trial_cache_lines STREQUAL "")
        file(CONFIGURE OUTPUT "${_whatson_trial_cache_script}" CONTENT "${_whatson_trial_cache_lines}" @ONLY)
    endif ()

    set(_whatson_trial_configure_command
            "${CMAKE_COMMAND}")
    if (NOT _whatson_trial_cache_lines STREQUAL "")
        list(APPEND _whatson_trial_configure_command -C "${_whatson_trial_cache_script}")
    endif ()
    list(APPEND _whatson_trial_configure_command
            -G "${CMAKE_GENERATOR}")
    if (CMAKE_GENERATOR_PLATFORM)
        list(APPEND _whatson_trial_configure_command -A "${CMAKE_GENERATOR_PLATFORM}")
    endif ()
    if (CMAKE_GENERATOR_TOOLSET)
        list(APPEND _whatson_trial_configure_command -T "${CMAKE_GENERATOR_TOOLSET}")
    endif ()
    if (CMAKE_GENERATOR_INSTANCE)
        list(APPEND _whatson_trial_configure_command
                "-DCMAKE_GENERATOR_INSTANCE=${CMAKE_GENERATOR_INSTANCE}")
    endif ()

    list(APPEND _whatson_trial_configure_command
            -S "${CMAKE_SOURCE_DIR}"
            -B "${WHATSON_TRIAL_BUILD_DIR}"
            "-DWHATSON_IS_TRIAL_BUILD=ON"
            "-DWHATSON_ENABLE_TRIAL_BUILD_MIRROR=OFF"
            "-DWHATSON_TRIAL_BUILD_DIR=${WHATSON_TRIAL_BUILD_DIR}"
            "-DWHATSON_ENABLE_IOS_XCODEPROJ_ON_BUILD=OFF"
            "-DWHATSON_BUILD_APP=${WHATSON_BUILD_APP}"
            "-DWHATSON_BUILD_DAEMON=${WHATSON_BUILD_DAEMON}"
            "-DWHATSON_BUILD_CLI=${WHATSON_BUILD_CLI}"
            "-DWHATSON_ENABLE_DEV_TOOLING=OFF"
            "-DWHATSON_IOS_SIMULATOR_ARCH=${WHATSON_IOS_SIMULATOR_ARCH}")

    if (DEFINED CMAKE_BUILD_TYPE AND NOT CMAKE_BUILD_TYPE STREQUAL "")
        list(APPEND _whatson_trial_configure_command "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}")
    endif ()
    if (DEFINED CMAKE_MAKE_PROGRAM AND NOT CMAKE_MAKE_PROGRAM STREQUAL "")
        list(APPEND _whatson_trial_configure_command "-DCMAKE_MAKE_PROGRAM=${CMAKE_MAKE_PROGRAM}")
    endif ()
    if (DEFINED QT_ROOT_PATH AND NOT QT_ROOT_PATH STREQUAL "")
        list(APPEND _whatson_trial_configure_command "-DQT_ROOT_PATH=${QT_ROOT_PATH}")
    endif ()
    if (DEFINED LVRS_PREFIX AND NOT LVRS_PREFIX STREQUAL "")
        list(APPEND _whatson_trial_configure_command "-DLVRS_PREFIX=${LVRS_PREFIX}")
    endif ()
    if (DEFINED LVRS_DIR AND NOT LVRS_DIR STREQUAL "")
        list(APPEND _whatson_trial_configure_command "-DLVRS_DIR=${LVRS_DIR}")
    endif ()
    if (APPLE)
        if (DEFINED CMAKE_OSX_DEPLOYMENT_TARGET AND NOT CMAKE_OSX_DEPLOYMENT_TARGET STREQUAL "")
            list(APPEND _whatson_trial_configure_command
                    "-DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET}")
        endif ()
        if (DEFINED CMAKE_OSX_SYSROOT AND NOT CMAKE_OSX_SYSROOT STREQUAL "")
            list(APPEND _whatson_trial_configure_command "-DCMAKE_OSX_SYSROOT=${CMAKE_OSX_SYSROOT}")
        endif ()
    endif ()

    set(_whatson_trial_build_command
            "${CMAKE_COMMAND}"
            --build "${WHATSON_TRIAL_BUILD_DIR}"
            --target whatson_package)
    if (CMAKE_CONFIGURATION_TYPES)
        list(APPEND _whatson_trial_build_command --config "$<CONFIG>")
    endif ()

    add_custom_target(whatson_sync_trial_build
            COMMAND "${CMAKE_COMMAND}" -E make_directory "${WHATSON_TRIAL_BUILD_DIR}"
            COMMAND ${_whatson_trial_configure_command}
            COMMAND ${_whatson_trial_build_command}
            USES_TERMINAL
            COMMENT "Configure and build the dedicated build-trial packaging tree"
    )
endif ()
