if (WHATSON_ENABLE_IOS_XCODEPROJ_ON_BUILD AND APPLE AND NOT CMAKE_SYSTEM_NAME STREQUAL "iOS" AND TARGET WhatSon)
    find_package(Python3 COMPONENTS Interpreter REQUIRED)

    set(_whatson_supported_ios_sdks iphonesimulator iphoneos)
    list(FIND _whatson_supported_ios_sdks "${WHATSON_IOS_SDK}" _whatson_ios_sdk_index)
    if (_whatson_ios_sdk_index EQUAL -1)
        message(FATAL_ERROR
                "Unsupported WHATSON_IOS_SDK value for whatson_generate_ios_xcodeproj: ${WHATSON_IOS_SDK}. "
                "Expected one of: ${_whatson_supported_ios_sdks}.")
    endif ()

    set(_whatson_ios_export_sdk "${WHATSON_IOS_SDK}")
    set(_whatson_ios_export_architectures "${WHATSON_IOS_ARCHITECTURES}")
    if (_whatson_ios_export_architectures STREQUAL "")
        if (_whatson_ios_export_sdk STREQUAL "iphonesimulator")
            set(_whatson_ios_export_architectures "${WHATSON_IOS_SIMULATOR_ARCH}")
        else ()
            set(_whatson_ios_export_architectures "arm64")
        endif ()
    endif ()

    set(_whatson_qt_ios_prefix "")
    if (DEFINED WHATSON_QT_IOS_PREFIX AND NOT WHATSON_QT_IOS_PREFIX STREQUAL "")
        set(_whatson_qt_ios_prefix "${WHATSON_QT_IOS_PREFIX}")
    elseif (DEFINED ENV{QT_IOS_PREFIX} AND NOT "$ENV{QT_IOS_PREFIX}" STREQUAL "")
        set(_whatson_qt_ios_prefix "$ENV{QT_IOS_PREFIX}")
    elseif (QT_ROOT_PATH AND EXISTS "${QT_ROOT_PATH}")
        get_filename_component(_whatson_qt_root_parent "${QT_ROOT_PATH}" DIRECTORY)
        if (EXISTS "${_whatson_qt_root_parent}/ios")
            set(_whatson_qt_ios_prefix "${_whatson_qt_root_parent}/ios")
        endif ()
        unset(_whatson_qt_root_parent)
    endif ()
    if (NOT _whatson_qt_ios_prefix STREQUAL "")
        set(WHATSON_QT_IOS_PREFIX "${_whatson_qt_ios_prefix}" CACHE PATH
                "Qt iOS prefix used for iOS Xcode project generation.")
    endif ()

    set(_whatson_lvrs_ios_prefix "")
    if (DEFINED WHATSON_LVRS_IOS_PREFIX AND NOT WHATSON_LVRS_IOS_PREFIX STREQUAL "")
        set(_whatson_lvrs_ios_prefix "${WHATSON_LVRS_IOS_PREFIX}")
    elseif (LVRS_PREFIX AND EXISTS "${LVRS_PREFIX}/platforms/ios")
        set(_whatson_lvrs_ios_prefix "${LVRS_PREFIX}/platforms/ios")
    elseif (DEFINED ENV{LVRS_IOS_PREFIX} AND NOT "$ENV{LVRS_IOS_PREFIX}" STREQUAL "")
        set(_whatson_lvrs_ios_prefix "$ENV{LVRS_IOS_PREFIX}")
    endif ()
    if (NOT _whatson_lvrs_ios_prefix STREQUAL "")
        set(WHATSON_LVRS_IOS_PREFIX "${_whatson_lvrs_ios_prefix}" CACHE PATH
                "LVRS iOS prefix used for iOS Xcode project generation.")
    endif ()

    set(_whatson_ios_local_package_flags)
    foreach (_whatson_ios_local_package IN ITEMS
            "iiXml;IIXML_PREFIX;WHATSON_IIXML_IOS_PREFIX;iiXml_DIR"
            "iiHtmlBlock;IIHTMLBLOCK_PREFIX;WHATSON_IIHTMLBLOCK_IOS_PREFIX;iiHtmlBlock_DIR")
        list(GET _whatson_ios_local_package 0 _whatson_ios_package_name)
        list(GET _whatson_ios_local_package 1 _whatson_ios_host_prefix_var)
        list(GET _whatson_ios_local_package 2 _whatson_ios_prefix_var)
        list(GET _whatson_ios_local_package 3 _whatson_ios_package_dir_var)

        set(_whatson_ios_package_prefix "${${_whatson_ios_prefix_var}}")
        if (_whatson_ios_package_prefix STREQUAL ""
                AND DEFINED ${_whatson_ios_host_prefix_var}
                AND EXISTS "${${_whatson_ios_host_prefix_var}}/platforms/ios")
            set(_whatson_ios_package_prefix "${${_whatson_ios_host_prefix_var}}/platforms/ios")
        endif ()
        if (_whatson_ios_package_prefix STREQUAL ""
                AND EXISTS "${WHATSON_LOCAL_LIBRARY_ROOT}/${_whatson_ios_package_name}/platforms/ios")
            set(_whatson_ios_package_prefix "${WHATSON_LOCAL_LIBRARY_ROOT}/${_whatson_ios_package_name}/platforms/ios")
        endif ()

        set(_whatson_ios_package_config_dir "")
        if (NOT _whatson_ios_package_prefix STREQUAL "")
            foreach (_whatson_ios_package_config_candidate IN ITEMS
                    "${_whatson_ios_package_prefix}/lib/cmake/${_whatson_ios_package_name}"
                    "${_whatson_ios_package_prefix}/blueprint/cmake/${_whatson_ios_package_name}")
                if (EXISTS "${_whatson_ios_package_config_candidate}/${_whatson_ios_package_name}Config.cmake")
                    set(_whatson_ios_package_config_dir "${_whatson_ios_package_config_candidate}")
                    break()
                endif ()
            endforeach ()
        endif ()

        if (NOT _whatson_ios_package_config_dir STREQUAL "")
            set("${_whatson_ios_prefix_var}" "${_whatson_ios_package_prefix}" CACHE PATH
                    "iOS ${_whatson_ios_package_name} prefix used for iOS Xcode project generation." FORCE)
            list(APPEND _whatson_ios_local_package_flags
                    "-D${_whatson_ios_prefix_var}=${_whatson_ios_package_prefix}"
                    "-D${_whatson_ios_host_prefix_var}=${_whatson_ios_package_prefix}"
                    "-D${_whatson_ios_package_dir_var}=${_whatson_ios_package_config_dir}")
        elseif (DEFINED ${_whatson_ios_prefix_var} AND NOT "${${_whatson_ios_prefix_var}}" STREQUAL "")
            list(APPEND _whatson_ios_local_package_flags
                    "-D${_whatson_ios_prefix_var}=${${_whatson_ios_prefix_var}}")
        endif ()
    endforeach ()

    if (NOT DEFINED WHATSON_QT_IOS_PREFIX OR WHATSON_QT_IOS_PREFIX STREQUAL "")
        message(WARNING "WHATSON_ENABLE_IOS_XCODEPROJ_ON_BUILD is ON but WHATSON_QT_IOS_PREFIX could not be resolved.")
    else ()
        set(_whatson_ios_toolchain "${WHATSON_QT_IOS_PREFIX}/lib/cmake/Qt6/qt.toolchain.cmake")
        if (NOT EXISTS "${_whatson_ios_toolchain}")
            message(WARNING "Qt iOS toolchain file was not found: ${_whatson_ios_toolchain}")
        else ()
            # Qt's optional gRPC / Protobuf Quick plugin packages try to resolve host protoc during
            # cross-compiling even though WhatSon does not use those modules.
            set(_whatson_ios_optional_qml_package_flags
                    -DCMAKE_DISABLE_FIND_PACKAGE_Qt6GrpcQuick=TRUE
                    -DCMAKE_DISABLE_FIND_PACKAGE_Qt6ProtobufQuick=TRUE
            )
            set(_whatson_ios_xcode_attribute_flags
                    "-DCMAKE_XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER=${WHATSON_APPLE_BUNDLE_ID}"
            )
            if (_whatson_ios_export_sdk STREQUAL "iphonesimulator")
                list(APPEND _whatson_ios_xcode_attribute_flags
                        -DCMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_ALLOWED=NO
                        -DCMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED=NO
                )
            else ()
                if (DEFINED WHATSON_IOS_CODE_SIGN_STYLE AND NOT WHATSON_IOS_CODE_SIGN_STYLE STREQUAL "")
                    list(APPEND _whatson_ios_xcode_attribute_flags
                            "-DCMAKE_XCODE_ATTRIBUTE_CODE_SIGN_STYLE=${WHATSON_IOS_CODE_SIGN_STYLE}")
                endif ()
                if (DEFINED WHATSON_IOS_DEVELOPMENT_TEAM AND NOT WHATSON_IOS_DEVELOPMENT_TEAM STREQUAL "")
                    list(APPEND _whatson_ios_xcode_attribute_flags
                            "-DCMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM=${WHATSON_IOS_DEVELOPMENT_TEAM}")
                else ()
                    message(WARNING
                            "WHATSON_IOS_SDK=iphoneos but WHATSON_IOS_DEVELOPMENT_TEAM is empty. "
                            "The generated Xcode project will still require a signing team selection "
                            "before it can be installed on a physical device.")
                endif ()
                if (DEFINED WHATSON_IOS_CODE_SIGN_IDENTITY AND NOT WHATSON_IOS_CODE_SIGN_IDENTITY STREQUAL "")
                    list(APPEND _whatson_ios_xcode_attribute_flags
                            "-DCMAKE_XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY=${WHATSON_IOS_CODE_SIGN_IDENTITY}")
                endif ()
            endif ()
            set(_whatson_ios_configure_cmd
                    "${CMAKE_COMMAND}"
                    -G Xcode
                    -S "${CMAKE_SOURCE_DIR}"
                    -B "${WHATSON_IOS_XCODEPROJ_DIR}"
                    -DCMAKE_SYSTEM_NAME=iOS
                    "-DCMAKE_TOOLCHAIN_FILE=${_whatson_ios_toolchain}"
                    -DCMAKE_BUILD_TYPE=Release
                    "-DCMAKE_OSX_SYSROOT=${_whatson_ios_export_sdk}"
                    "-DCMAKE_OSX_ARCHITECTURES=${_whatson_ios_export_architectures}"
                    "-DWHATSON_IOS_SDK=${_whatson_ios_export_sdk}"
                    "-DWHATSON_IOS_ARCHITECTURES=${_whatson_ios_export_architectures}"
                    "-DWHATSON_IOS_DEVELOPMENT_TEAM=${WHATSON_IOS_DEVELOPMENT_TEAM}"
                    "-DWHATSON_IOS_CODE_SIGN_IDENTITY=${WHATSON_IOS_CODE_SIGN_IDENTITY}"
                    "-DWHATSON_IOS_CODE_SIGN_STYLE=${WHATSON_IOS_CODE_SIGN_STYLE}"
                    "-DWHATSON_IOS_QT_PERMISSION_PLUGIN_POLICY=${WHATSON_IOS_QT_PERMISSION_PLUGIN_POLICY}"
                    "-DWHATSON_PLATFORM_DIR=${WHATSON_PLATFORM_DIR}"
                    "-DWHATSON_APPLE_DIR=${WHATSON_APPLE_DIR}"
                    "-DWHATSON_IOS_INFO_PLIST=${WHATSON_IOS_INFO_PLIST}"
                    "-DWHATSON_DEFAULT_INFO_PLIST=${WHATSON_DEFAULT_INFO_PLIST}"
                    "-DWHATSON_ENTITLEMENTS=${WHATSON_ENTITLEMENTS}"
                    "-DWHATSON_APPLE_BUNDLE_ID=${WHATSON_APPLE_BUNDLE_ID}"
                    "-DWHATSON_ANDROID_PACKAGE_ID=${WHATSON_ANDROID_PACKAGE_ID}"
                    ${_whatson_ios_local_package_flags}
                    ${_whatson_ios_optional_qml_package_flags}
                    ${_whatson_ios_xcode_attribute_flags}
            )

            if (DEFINED WHATSON_LVRS_IOS_PREFIX AND EXISTS "${WHATSON_LVRS_IOS_PREFIX}/lib/cmake/LVRS")
                list(APPEND _whatson_ios_configure_cmd
                        "-DLVRS_DIR=${WHATSON_LVRS_IOS_PREFIX}/lib/cmake/LVRS")
            endif ()

            set(_whatson_prepare_ios_export_script
                    "${CMAKE_BINARY_DIR}/CMakeFiles/PrepareWhatSonIosXcodeproj.cmake")
            configure_file(
                    "${CMAKE_SOURCE_DIR}/cmake/PrepareWhatSonIosXcodeproj.cmake.in"
                    "${_whatson_prepare_ios_export_script}"
                    @ONLY
            )

            set(_whatson_ios_xcodeproj_patch_args
                    "${WHATSON_IOS_XCODEPROJ_DIR}/WhatSon.xcodeproj/project.pbxproj")
            if (_whatson_ios_export_sdk STREQUAL "iphonesimulator"
                    AND NOT WHATSON_IOS_QT_PERMISSION_PLUGIN_POLICY STREQUAL "include")
                list(APPEND _whatson_ios_xcodeproj_patch_args --strip-qt-permissions)
            endif ()

            add_custom_target(whatson_generate_ios_xcodeproj
                    COMMAND "${CMAKE_COMMAND}" -P "${_whatson_prepare_ios_export_script}"
                    COMMAND ${_whatson_ios_configure_cmd}
                    COMMAND "${Python3_EXECUTABLE}"
                    "${CMAKE_SOURCE_DIR}/cmake/patch_whatson_ios_xcodeproj.py"
                    ${_whatson_ios_xcodeproj_patch_args}
                    BYPRODUCTS "${WHATSON_IOS_XCODEPROJ_DIR}/WhatSon.xcodeproj"
                    USES_TERMINAL
                    COMMENT "Generate iOS Xcode project from platform/Apple metadata"
            )
        endif ()
    endif ()
endif ()

if (TARGET WhatSon)
    add_custom_target(whatson_run_app
            COMMAND "$<TARGET_FILE:WhatSon>"
            DEPENDS WhatSon
            USES_TERMINAL
            COMMENT "Run WhatSon app on current host"
    )
endif ()

if (TARGET whatson_cli AND DEFINED WHATSON_CLI_BINARY_PATH AND NOT WHATSON_CLI_BINARY_PATH STREQUAL "")
    add_custom_target(whatson_run_cli
            COMMAND "${WHATSON_CLI_BINARY_PATH}"
            DEPENDS whatson_cli
            USES_TERMINAL
            COMMENT "Run Rust CLI launcher (whatson)"
    )
endif ()

if (TARGET WhatSon_daemon)
    add_custom_target(whatson_run_daemon
            COMMAND "$<TARGET_FILE:WhatSon_daemon>"
            DEPENDS WhatSon_daemon
            USES_TERMINAL
            COMMENT "Run WhatSon daemon on current host"
    )

    add_custom_target(whatson_healthcheck_daemon
            COMMAND "$<TARGET_FILE:WhatSon_daemon>" --healthcheck
            DEPENDS WhatSon_daemon
            USES_TERMINAL
            COMMENT "Run daemon healthcheck"
    )
endif ()

if (TARGET launch_WhatSon_ios)
    add_custom_target(whatson_launch_ios DEPENDS launch_WhatSon_ios)
endif ()

if (TARGET launch_WhatSon_android)
    add_custom_target(whatson_launch_android DEPENDS launch_WhatSon_android)
endif ()

if (TARGET launch_WhatSon_wasm)
    add_custom_target(whatson_launch_wasm DEPENDS launch_WhatSon_wasm)
endif ()

if (TARGET whatson_generate_ios_xcodeproj)
    add_custom_target(whatson_export_xcodeproj DEPENDS whatson_generate_ios_xcodeproj)
elseif (TARGET export_WhatSon_xcodeproj)
    add_custom_target(whatson_export_xcodeproj DEPENDS export_WhatSon_xcodeproj)
endif ()

if (TARGET export_WhatSon_android_studio)
    add_custom_target(whatson_export_android_studio DEPENDS export_WhatSon_android_studio)
endif ()

if (TARGET export_WhatSon_wasm_site)
    add_custom_target(whatson_export_wasm_site DEPENDS export_WhatSon_wasm_site)
endif ()
