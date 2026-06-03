#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${BUILD_DIR:-${ROOT_DIR}/build}"
TRIAL_BUILD_DIR="${TRIAL_BUILD_DIR:-${ROOT_DIR}/build-trial}"
APP_TARGET="${APP_TARGET:-WhatSon}"
PLATFORM="${1:-host}"
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

log() {
    printf '[bootstrap] %s\n' "$*"
}

die_usage() {
    printf 'Usage: %s [host|macos|linux]\n' "$(basename "$0")" >&2
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

ensure_requested_platform_matches_host() {
    local requested="$1"
    case "${requested}" in
        host)
            return 0
            ;;
        macos)
            [[ "${HOST_PLATFORM}" == "macos" ]] && return 0
            ;;
        linux)
            [[ "${HOST_PLATFORM}" == "linux" ]] && return 0
            ;;
    esac

    printf 'Requested platform "%s" is not supported on host "%s".\n' "${requested}" "${HOST_PLATFORM}" >&2
    exit 2
}

if [[ "${PLATFORM}" != "host" && "${PLATFORM}" != "macos" && "${PLATFORM}" != "linux" ]]; then
    die_usage
fi

ensure_requested_platform_matches_host "${PLATFORM}"

if [[ -z "${QT_VERSION_ROOT}" ]]; then
    QT_VERSION_ROOT="$(detect_qt_version_root)"
fi

QT_HOST_PREFIX="${QT_HOST_PREFIX:-$(default_qt_host_prefix)}"

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
    )
    if [[ -n "${prefix_path}" ]]; then
        cmake_args+=("-DCMAKE_PREFIX_PATH=${prefix_path}")
    fi

    if [[ "${HOST_PLATFORM}" == "macos" ]]; then
        cmake_args+=(-DLVRS_BOOTSTRAP_QT_PREFIX_MACOS="${QT_HOST_PREFIX}")
    elif [[ "${HOST_PLATFORM}" == "linux" ]]; then
        cmake_args+=(-DLVRS_BOOTSTRAP_QT_PREFIX_LINUX="${QT_HOST_PREFIX}")
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
    )
    if [[ -n "${prefix_path}" ]]; then
        cmake_args+=("-DCMAKE_PREFIX_PATH=${prefix_path}")
    fi

    if [[ "${HOST_PLATFORM}" == "macos" ]]; then
        cmake_args+=(-DLVRS_BOOTSTRAP_QT_PREFIX_MACOS="${QT_HOST_PREFIX}")
    elif [[ "${HOST_PLATFORM}" == "linux" ]]; then
        cmake_args+=(-DLVRS_BOOTSTRAP_QT_PREFIX_LINUX="${QT_HOST_PREFIX}")
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

    log "Building target: ${target_name}"
    set +e
    cmake --build "${BUILD_DIR}" --target "${target_name}" -j 2>&1 | tee "${log_file}"
    result=${PIPESTATUS[0]}
    set -e
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

log "Host platform: ${HOST_PLATFORM}"
log "Qt host prefix: ${QT_HOST_PREFIX}"
log "LVRS prefix: ${LVRS_PREFIX}"

configure_root
configure_trial_root

overall_result=0
build_platform "${HOST_PLATFORM}" || overall_result=1
build_trial_package || overall_result=1

exit ${overall_result}
