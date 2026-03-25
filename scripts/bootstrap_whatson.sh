#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${BUILD_DIR:-${ROOT_DIR}/build}"
TRIAL_BUILD_DIR="${TRIAL_BUILD_DIR:-${ROOT_DIR}/build-trial}"
APP_TARGET="${APP_TARGET:-WhatSon}"
PLATFORM="${1:-all}"
LVRS_PREFIX="${LVRS_PREFIX:-${HOME}/.local/LVRS}"
QT_VERSION_ROOT="${QT_VERSION_ROOT:-}"

HOST_UNAME="$(uname -s)"

detect_host_platform() {
    case "${HOST_UNAME}" in
        Darwin)
            printf 'macos'
            ;;
        Linux)
            printf 'linux'
            ;;
        *)
            printf 'Unsupported host platform: %s\n' "${HOST_UNAME}" >&2
            exit 2
            ;;
    esac
}

HOST_PLATFORM="$(detect_host_platform)"

default_android_sdk_root() {
    case "${HOST_PLATFORM}" in
        macos)
            printf '%s' "${HOME}/Library/Android/sdk"
            ;;
        linux)
            printf '%s' "${HOME}/Android/Sdk"
            ;;
    esac
}

ANDROID_SDK_ROOT="${ANDROID_SDK_ROOT:-$(default_android_sdk_root)}"
export ANDROID_SDK_ROOT
export ANDROID_HOME="${ANDROID_HOME:-${ANDROID_SDK_ROOT}}"

log() {
    printf '[bootstrap] %s\n' "$*"
}

die_usage() {
    printf 'Usage: %s [all|host|macos|linux|ios|android]\n' "$(basename "$0")" >&2
    exit 2
}

join_by_semicolon() {
    local first=1
    local value
    for value in "$@"; do
        if [[ -z "${value}" ]]; then
            continue
        fi
        if [[ ${first} -eq 1 ]]; then
            printf '%s' "${value}"
            first=0
        else
            printf ';%s' "${value}"
        fi
    done
}

detect_qt_version_root() {
    local qt_home
    local candidate
    qt_home="${HOME}/Qt"

    if [[ -d "${qt_home}" ]]; then
        candidate="$(
            find "${qt_home}" -mindepth 1 -maxdepth 1 -type d \
                | awk -F/ '{print $NF}' \
                | grep -E '^[0-9]+(\.[0-9]+)*$' \
                | awk -F. '{ printf "%04d%04d%04d%04d %s\n", $1, $2, $3, $4, $0 }' \
                | sort \
                | tail -n 1 \
                | awk '{print $2}' \
                || true
        )"
        if [[ -n "${candidate}" ]]; then
            printf '%s' "${qt_home}/${candidate}"
            return 0
        fi
    fi

    printf '%s' "${qt_home}"
}

default_qt_host_prefix() {
    case "${HOST_PLATFORM}" in
        macos)
            printf '%s' "${QT_VERSION_ROOT}/macos"
            ;;
        linux)
            printf '%s' "${QT_VERSION_ROOT}/gcc_64"
            ;;
    esac
}

resolve_lvrs_dir() {
    local prefix="$1"
    local platform_name="$2"
    local candidate

    for candidate in \
        "${prefix}/platforms/${platform_name}/lib/cmake/LVRS" \
        "${prefix}/platforms/${platform_name}/blueprint/cmake/LVRS" \
        "${prefix}/lib/cmake/LVRS" \
        "${prefix}/blueprint/cmake/LVRS"; do
        if [[ -d "${candidate}" ]]; then
            printf '%s' "${candidate}"
            return 0
        fi
    done

    printf '%s' "${prefix}/lib/cmake/LVRS"
}

detect_ios_simulator_arch() {
    local probe
    local arm64_platform
    probe="${QT_IOS_PREFIX}/qml/QML/objects-Release/Qml_resources_1/.qt/rcc/qrc_qmake_QML_init.cpp.o"
    if [[ ! -f "${probe}" ]] || ! command -v xcrun >/dev/null 2>&1; then
        printf 'arm64'
        return 0
    fi

    arm64_platform="$(
        xcrun vtool -show-build "${probe}" 2>/dev/null | awk '
            /architecture arm64/ { in_arm=1; next }
            in_arm && /architecture / { in_arm=0 }
            in_arm && /platform / { print $2; exit }
        '
    )"

    if [[ "${arm64_platform}" == "IOS" ]]; then
        if [[ "${WHATSON_IOS_ALLOW_X86_FALLBACK:-0}" == "1" ]]; then
            printf 'x86_64'
            return 0
        fi
    fi

    printf 'arm64'
}

detect_android_ndk() {
    local candidate
    if [[ -n "${ANDROID_NDK_ROOT:-}" && -d "${ANDROID_NDK_ROOT}" ]]; then
        printf '%s' "${ANDROID_NDK_ROOT}"
        return 0
    fi

    if [[ -d "${ANDROID_SDK_ROOT}/ndk" ]]; then
        candidate="$(find "${ANDROID_SDK_ROOT}/ndk" -mindepth 1 -maxdepth 1 -type d | sort -r | head -n 1 || true)"
        if [[ -n "${candidate}" ]]; then
            printf '%s' "${candidate}"
            return 0
        fi
    fi

    printf ''
}

ensure_requested_platform_matches_host() {
    local requested="$1"
    case "${requested}" in
        host|all|android)
            return 0
            ;;
        macos)
            [[ "${HOST_PLATFORM}" == "macos" ]] && return 0
            ;;
        linux)
            [[ "${HOST_PLATFORM}" == "linux" ]] && return 0
            ;;
        ios)
            [[ "${HOST_PLATFORM}" == "macos" ]] && return 0
            ;;
    esac

    printf 'Requested platform "%s" is not supported on host "%s".\n' "${requested}" "${HOST_PLATFORM}" >&2
    exit 2
}

if [[ -z "${QT_VERSION_ROOT}" ]]; then
    QT_VERSION_ROOT="$(detect_qt_version_root)"
fi

QT_HOST_PREFIX="${QT_HOST_PREFIX:-$(default_qt_host_prefix)}"
QT_IOS_PREFIX="${QT_IOS_PREFIX:-${QT_VERSION_ROOT}/ios}"
QT_ANDROID_PREFIX="${QT_ANDROID_PREFIX:-${QT_VERSION_ROOT}/android_arm64_v8a}"

configure_root() {
    local prefix_path
    local lvrs_dir
    local cmake_args=()
    local prefix_args=()

    log "Configuring build dir: ${BUILD_DIR}"

    if [[ -d "${QT_HOST_PREFIX}" ]]; then
        prefix_args+=("${QT_HOST_PREFIX}")
    fi
    if [[ -d "${LVRS_PREFIX}" ]]; then
        prefix_args+=("${LVRS_PREFIX}")
    fi

    prefix_path="$(join_by_semicolon "${prefix_args[@]}")"
    lvrs_dir="$(resolve_lvrs_dir "${LVRS_PREFIX}" "${HOST_PLATFORM}")"

    cmake_args=(
        -S "${ROOT_DIR}"
        -B "${BUILD_DIR}"
        -DLVRS_DIR="${lvrs_dir}"
        -DCMAKE_FIND_PACKAGE_NO_PACKAGE_REGISTRY=ON
        -DQT_ROOT_PATH="${QT_HOST_PREFIX}"
        -DLVRS_BOOTSTRAP_QT_PREFIX_ANDROID="${QT_ANDROID_PREFIX}"
    )
    if [[ -n "${prefix_path}" ]]; then
        cmake_args+=("-DCMAKE_PREFIX_PATH=${prefix_path}")
    fi

    if [[ "${HOST_PLATFORM}" == "macos" ]]; then
        cmake_args+=(
            -DLVRS_BOOTSTRAP_QT_PREFIX_MACOS="${QT_HOST_PREFIX}"
            -DLVRS_BOOTSTRAP_QT_PREFIX_IOS="${QT_IOS_PREFIX}"
            -DWHATSON_ENABLE_IOS_XCODEPROJ_ON_BUILD=ON
            -DLVRS_BOOTSTRAP_IOS_ARCHITECTURES="${IOS_SIMULATOR_ARCH}"
        )
    elif [[ "${HOST_PLATFORM}" == "linux" ]]; then
        cmake_args+=(
            -DLVRS_BOOTSTRAP_QT_PREFIX_LINUX="${QT_HOST_PREFIX}"
            -DWHATSON_ENABLE_IOS_XCODEPROJ_ON_BUILD=OFF
        )
    fi

    cmake "${cmake_args[@]}"
}

configure_trial_root() {
    local prefix_path
    local lvrs_dir
    local cmake_args=()
    local prefix_args=()

    log "Configuring trial packaging build dir: ${TRIAL_BUILD_DIR}"

    if [[ -d "${QT_HOST_PREFIX}" ]]; then
        prefix_args+=("${QT_HOST_PREFIX}")
    fi
    if [[ -d "${LVRS_PREFIX}" ]]; then
        prefix_args+=("${LVRS_PREFIX}")
    fi

    prefix_path="$(join_by_semicolon "${prefix_args[@]}")"
    lvrs_dir="$(resolve_lvrs_dir "${LVRS_PREFIX}" "${HOST_PLATFORM}")"

    cmake_args=(
        -S "${ROOT_DIR}"
        -B "${TRIAL_BUILD_DIR}"
        -DLVRS_DIR="${lvrs_dir}"
        -DCMAKE_FIND_PACKAGE_NO_PACKAGE_REGISTRY=ON
        -DQT_ROOT_PATH="${QT_HOST_PREFIX}"
        -DLVRS_BOOTSTRAP_QT_PREFIX_ANDROID="${QT_ANDROID_PREFIX}"
    )
    if [[ -n "${prefix_path}" ]]; then
        cmake_args+=("-DCMAKE_PREFIX_PATH=${prefix_path}")
    fi

    if [[ "${HOST_PLATFORM}" == "macos" ]]; then
        cmake_args+=(
            -DLVRS_BOOTSTRAP_QT_PREFIX_MACOS="${QT_HOST_PREFIX}"
            -DLVRS_BOOTSTRAP_QT_PREFIX_IOS="${QT_IOS_PREFIX}"
            -DWHATSON_ENABLE_IOS_XCODEPROJ_ON_BUILD=OFF
            -DLVRS_BOOTSTRAP_IOS_ARCHITECTURES="${IOS_SIMULATOR_ARCH}"
        )
    elif [[ "${HOST_PLATFORM}" == "linux" ]]; then
        cmake_args+=(
            -DLVRS_BOOTSTRAP_QT_PREFIX_LINUX="${QT_HOST_PREFIX}"
            -DWHATSON_ENABLE_IOS_XCODEPROJ_ON_BUILD=OFF
        )
    fi

    cmake "${cmake_args[@]}"
}

build_platform() {
    local platform_name="$1"
    local target_name
    local log_file
    local result

    target_name="bootstrap_${APP_TARGET}_${platform_name}"
    log_file="${BUILD_DIR}/${target_name}.log"

    if [[ "${platform_name}" == "ios" ]]; then
        cmake -E remove_directory "${BUILD_DIR}/lvrs-bootstrap/${APP_TARGET}/ios"
    fi

    log "Building target: ${target_name}"
    set +e
    cmake --build "${BUILD_DIR}" --target "${target_name}" -j 2>&1 | tee "${log_file}"
    result=${PIPESTATUS[0]}
    set -e

    if [[ ${result} -ne 0 && "${platform_name}" == "ios" ]]; then
        if grep -q "building for 'iOS-simulator'.*built for 'iOS'" "${log_file}"; then
            log "Qt iOS kit has arm64 objects tagged for iOS device, not iOS simulator."
            log "Fix by updating/reinstalling Qt iOS kit with arm64 simulator-compatible slices."
            log "Temporary fallback: WHATSON_IOS_ALLOW_X86_FALLBACK=1 ./scripts/bootstrap_whatson.sh ios"
        fi
        if [[ "${IOS_SIMULATOR_ARCH}" == "x86_64" ]] && grep -q "Needs to Be Updated" "${log_file}"; then
            log "iOS simulator rejected x86_64 app. Current simulator runtime requires arm64."
        fi
    fi

    return ${result}
}

build_trial_package() {
    local log_file
    local result

    log_file="${TRIAL_BUILD_DIR}/whatson_package.log"
    log "Building trial package target: whatson_package"
    set +e
    cmake --build "${TRIAL_BUILD_DIR}" --target "whatson_package" -j 2>&1 | tee "${log_file}"
    result=${PIPESTATUS[0]}
    set -e
    return ${result}
}

if [[ "${PLATFORM}" != "all" && "${PLATFORM}" != "host" && "${PLATFORM}" != "macos" && "${PLATFORM}" != "linux" && "${PLATFORM}" != "ios" && "${PLATFORM}" != "android" ]]; then
    die_usage
fi

ensure_requested_platform_matches_host "${PLATFORM}"

IOS_SIMULATOR_ARCH="${LVRS_BOOTSTRAP_IOS_ARCHITECTURES:-$(detect_ios_simulator_arch)}"
ANDROID_NDK_ROOT_DETECTED="$(detect_android_ndk)"

if [[ -n "${ANDROID_NDK_ROOT_DETECTED}" ]]; then
    export ANDROID_NDK_ROOT="${ANDROID_NDK_ROOT_DETECTED}"
    export ANDROID_NDK="${ANDROID_NDK_ROOT_DETECTED}"
    export CMAKE_ANDROID_NDK="${ANDROID_NDK_ROOT_DETECTED}"
    export QT_ANDROID_NDK_ROOT="${ANDROID_NDK_ROOT_DETECTED}"
    log "Using Android NDK: ${ANDROID_NDK_ROOT_DETECTED}"
else
    log "Android NDK not found under ${ANDROID_SDK_ROOT}/ndk."
fi

log "Host platform: ${HOST_PLATFORM}"
log "Qt host prefix: ${QT_HOST_PREFIX}"
log "LVRS prefix: ${LVRS_PREFIX}"
if [[ "${HOST_PLATFORM}" == "macos" ]]; then
    log "iOS simulator arch: ${IOS_SIMULATOR_ARCH}"
    if [[ "${IOS_SIMULATOR_ARCH}" == "x86_64" ]]; then
        log "x86_64 fallback is enabled. This is temporary and may fail to install on modern simulators."
    fi
fi

configure_root

if [[ "${PLATFORM}" == "all" || "${PLATFORM}" == "host" || "${PLATFORM}" == "macos" || "${PLATFORM}" == "linux" ]]; then
    configure_trial_root
fi

overall_result=0

if [[ "${PLATFORM}" == "all" ]]; then
    build_platform "${HOST_PLATFORM}" || overall_result=1
    build_trial_package || overall_result=1

    if [[ "${HOST_PLATFORM}" == "macos" ]]; then
        build_platform "ios" || overall_result=1
    fi

    if [[ -n "${ANDROID_NDK_ROOT_DETECTED}" ]]; then
        build_platform "android" || overall_result=1
    else
        log "Skipping Android bootstrap because NDK is not installed."
    fi
elif [[ "${PLATFORM}" == "host" ]]; then
    build_platform "${HOST_PLATFORM}" || overall_result=1
    build_trial_package || overall_result=1
else
    if [[ "${PLATFORM}" == "android" && -z "${ANDROID_NDK_ROOT_DETECTED}" ]]; then
        log "Cannot build Android target without NDK."
        exit 2
    fi
    build_platform "${PLATFORM}" || overall_result=1
    if [[ "${PLATFORM}" == "macos" || "${PLATFORM}" == "linux" ]]; then
        build_trial_package || overall_result=1
    fi
fi

exit ${overall_result}
