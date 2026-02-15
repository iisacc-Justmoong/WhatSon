#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${BUILD_DIR:-${ROOT_DIR}/build}"
APP_TARGET="${APP_TARGET:-WhatSon}"
PLATFORM="${1:-all}"

QT_VERSION_ROOT="${QT_VERSION_ROOT:-${HOME}/Qt/6.8.3}"
QT_MACOS_PREFIX="${QT_MACOS_PREFIX:-${QT_VERSION_ROOT}/macos}"
QT_IOS_PREFIX="${QT_IOS_PREFIX:-${QT_VERSION_ROOT}/ios}"
QT_ANDROID_PREFIX="${QT_ANDROID_PREFIX:-${QT_VERSION_ROOT}/android_arm64_v8a}"
LVRS_PREFIX="${LVRS_PREFIX:-${HOME}/.local/LVRS}"

ANDROID_SDK_ROOT="${ANDROID_SDK_ROOT:-${HOME}/Library/Android/sdk}"
export ANDROID_SDK_ROOT
export ANDROID_HOME="${ANDROID_HOME:-${ANDROID_SDK_ROOT}}"

log() {
    printf '[bootstrap] %s\n' "$*"
}

die_usage() {
    printf 'Usage: %s [all|macos|ios|android]\n' "$(basename "$0")" >&2
    exit 2
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

configure_root() {
    log "Configuring build dir: ${BUILD_DIR}"
    cmake -S "${ROOT_DIR}" -B "${BUILD_DIR}" \
        -DCMAKE_PREFIX_PATH="${QT_MACOS_PREFIX};${LVRS_PREFIX}" \
        -DLVRS_DIR="${LVRS_PREFIX}/lib/cmake/LVRS" \
        -DCMAKE_FIND_PACKAGE_NO_PACKAGE_REGISTRY=ON \
        -DLVRS_BOOTSTRAP_QT_PREFIX_IOS="${QT_IOS_PREFIX}" \
        -DLVRS_BOOTSTRAP_QT_PREFIX_ANDROID="${QT_ANDROID_PREFIX}" \
        -DLVRS_BOOTSTRAP_IOS_ARCHITECTURES="${IOS_SIMULATOR_ARCH}"
}

clean_platform_bootstrap_dir() {
    local platform_name="$1"
    cmake -E remove_directory "${BUILD_DIR}/lvrs-bootstrap/${APP_TARGET}/${platform_name}"
}

build_platform() {
    local platform_name="$1"
    local target_name
    local log_file
    local result

    target_name="bootstrap_${APP_TARGET}_${platform_name}"
    log_file="${BUILD_DIR}/${target_name}.log"

    if [[ "${platform_name}" == "ios" ]]; then
        clean_platform_bootstrap_dir "ios"
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

if [[ "${PLATFORM}" != "all" && "${PLATFORM}" != "macos" && "${PLATFORM}" != "ios" && "${PLATFORM}" != "android" ]]; then
    die_usage
fi

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

log "iOS simulator arch: ${IOS_SIMULATOR_ARCH}"
if [[ "${IOS_SIMULATOR_ARCH}" == "x86_64" ]]; then
    log "x86_64 fallback is enabled. This is temporary and may fail to install on modern simulators."
fi

configure_root

overall_result=0

if [[ "${PLATFORM}" == "all" ]]; then
    build_platform "macos" || overall_result=1
    build_platform "ios" || overall_result=1

    if [[ -n "${ANDROID_NDK_ROOT_DETECTED}" ]]; then
        build_platform "android" || overall_result=1
    else
        log "Skipping Android bootstrap because NDK is not installed."
        overall_result=1
    fi
else
    if [[ "${PLATFORM}" == "android" && -z "${ANDROID_NDK_ROOT_DETECTED}" ]]; then
        log "Cannot build Android target without NDK."
        exit 2
    fi
    build_platform "${PLATFORM}" || overall_result=1
fi

exit ${overall_result}
