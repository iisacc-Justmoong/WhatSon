#!/usr/bin/env python3
from __future__ import annotations

import argparse
import json
import os
import platform
import re
import shlex
import shutil
import subprocess
import sys
import textwrap
import xml.etree.ElementTree as ET
from pathlib import Path
from typing import Dict, Iterable, List, Optional, Sequence, Tuple

DEFAULT_ANDROID_PACKAGE_ID = "com.lvrs.whatson"
DEFAULT_APPLE_BUNDLE_ID = "com.lvrs.whatson"


def _expand(value: str | Path) -> Path:
    return Path(os.path.expandvars(os.path.expanduser(str(value)))).resolve()


def _run_capture(cmd: Sequence[str]) -> subprocess.CompletedProcess[str]:
    return subprocess.run(
        [str(part) for part in cmd],
        capture_output=True,
        text=True,
        check=False,
    )


def _latest_version_dir(parent: Path, pattern: str = r"^\d+(\.\d+)*$") -> Optional[Path]:
    if not parent.is_dir():
        return None

    regex = re.compile(pattern)
    candidates = [item for item in parent.iterdir() if item.is_dir() and regex.match(item.name)]
    if not candidates:
        return None

    def sort_key(item: Path) -> Tuple[int, ...]:
        return tuple(int(part) for part in re.findall(r"\d+", item.name))

    return sorted(candidates, key=sort_key)[-1]


def _first_existing(paths: Iterable[Path]) -> Optional[Path]:
    for path in paths:
        if path.exists():
            return path
    return None


def _latest_ndk_dir(android_sdk_root: Path) -> Optional[Path]:
    ndk_root = android_sdk_root / "ndk"
    if not ndk_root.is_dir():
        return None
    versions = [item for item in ndk_root.iterdir() if item.is_dir()]
    if not versions:
        return None
    return sorted(versions, key=lambda item: item.name)[-1]


def _read_android_manifest_package(repo_root: Path) -> Optional[str]:
    candidates = [
        repo_root / "platform" / "Android" / "AndroidManifest.xml",
        repo_root / "platform" / "android" / "AndroidManifest.xml",
        repo_root / "android" / "AndroidManifest.xml",
    ]
    for manifest_path in candidates:
        if not manifest_path.exists():
            continue
        try:
            xml_root = ET.parse(manifest_path).getroot()
        except ET.ParseError:
            continue
        package_name = xml_root.attrib.get("package", "").strip()
        if package_name:
            return package_name
    return None


def _detect_avd_name() -> Optional[str]:
    env_avd = os.environ.get("ANDROID_AVD")
    if env_avd:
        return env_avd

    emulator_bin = shutil.which("emulator")
    if emulator_bin:
        probe = _run_capture([emulator_bin, "-list-avds"])
        if probe.returncode == 0:
            names = [line.strip() for line in probe.stdout.splitlines() if line.strip()]
            if names:
                return names[0]

    avd_root = _expand("~/.android/avd")
    if avd_root.is_dir():
        avd_files = sorted(avd_root.glob("*.ini"))
        for avd_ini in avd_files:
            if avd_ini.stem:
                return avd_ini.stem
    return None


def _default_android_sdk(system_name: str, home: Path) -> Path:
    if system_name == "Darwin":
        return home / "Library" / "Android" / "sdk"
    if system_name == "Windows":
        localappdata = os.environ.get("LOCALAPPDATA")
        if localappdata:
            return _expand(localappdata) / "Android" / "Sdk"
    return home / "Android" / "Sdk"


def _default_qt_host_prefix(system_name: str, qt_version_root: Path) -> Path:
    if system_name == "Darwin":
        return qt_version_root / "macos"
    if system_name == "Windows":
        return _first_existing(
            [
                qt_version_root / "msvc2022_64",
                qt_version_root / "msvc2019_64",
                qt_version_root / "mingw_64",
            ]
        ) or (qt_version_root / "msvc2022_64")
    return qt_version_root / "gcc_64"


def _default_qt_android_prefix(qt_version_root: Path) -> Path:
    return _first_existing(
        [
            qt_version_root / "android_arm64_v8a",
            qt_version_root / "android_x86_64",
            qt_version_root / "android",
        ]
    ) or (qt_version_root / "android_arm64_v8a")


def _default_qt_version_root(home: Path) -> Path:
    qt_home = home / "Qt"
    return _latest_version_dir(qt_home) or qt_home


def _default_lvrs_source_dir(home: Path, repo_root: Path) -> Optional[Path]:
    candidates = [
        home / "Developer" / "LVRS",
        repo_root.parent / "LVRS",
        repo_root.parent.parent / "LVRS",
        Path("/local/LVRS"),
    ]
    for path in candidates:
        if (path / "CMakeLists.txt").exists():
            return path
    return None


def _resolve_lvrs_prefix(home: Path, repo_root: Path) -> Path:
    env_prefix = os.environ.get("LVRS_PREFIX")
    if env_prefix:
        return _expand(env_prefix)

    candidates = [
        home / ".local" / "LVRS",
        Path("/local/LVRS"),
        repo_root.parent / "LVRS",
    ]
    return _first_existing(candidates) or candidates[0]


def _resolve_lvrs_dir(lvrs_prefix: Path, system_name: str) -> Path:
    root_dispatch = lvrs_prefix / "lib" / "cmake" / "LVRS"
    if root_dispatch.exists():
        return root_dispatch

    platform_key = {
        "Darwin": "macos",
        "Linux": "linux",
        "Windows": "windows",
    }.get(system_name, "macos")
    platform_dir = lvrs_prefix / "platforms" / platform_key / "lib" / "cmake" / "LVRS"
    if platform_dir.exists():
        return platform_dir

    return root_dispatch


def _resolve_lvrs_android_prefix(home: Path, lvrs_prefix: Path) -> Path:
    env_android_prefix = os.environ.get("LVRS_ANDROID_PREFIX")
    if env_android_prefix:
        return _expand(env_android_prefix)

    platform_android = lvrs_prefix / "platforms" / "android"
    if platform_android.exists():
        return platform_android
    fallback = home / ".local" / "LVRS-android"
    if fallback.exists():
        return fallback
    return platform_android


def _resolve_java21_home(system_name: str) -> Optional[Path]:
    env_java = os.environ.get("JAVA21_HOME")
    if env_java:
        return _expand(env_java)
    env_java_home = os.environ.get("JAVA_HOME")
    if env_java_home:
        return _expand(env_java_home)
    if system_name != "Darwin":
        return None

    java_home_cmd = Path("/usr/libexec/java_home")
    if java_home_cmd.exists():
        probe = _run_capture([str(java_home_cmd), "-v", "21"])
        if probe.returncode == 0:
            resolved = probe.stdout.strip()
            if resolved:
                resolved_path = Path(resolved)
                if resolved_path.exists():
                    return resolved_path

    candidates = [
        Path("/opt/homebrew/opt/openjdk@21/libexec/openjdk.jdk/Contents/Home"),
        Path("/Library/Java/JavaVirtualMachines/openjdk-21.jdk/Contents/Home"),
        Path("/Library/Java/JavaVirtualMachines/temurin-21.jdk/Contents/Home"),
    ]
    return _first_existing(candidates)


def _resolve_android_ndk(android_sdk_root: Path) -> Optional[Path]:
    env_ndk = os.environ.get("ANDROID_NDK_ROOT")
    if env_ndk:
        return _expand(env_ndk)

    ndk_dir = _latest_ndk_dir(android_sdk_root)
    if ndk_dir:
        return ndk_dir

    if platform.system() == "Darwin":
        cask_root = Path("/opt/homebrew/Caskroom/android-ndk")
        if cask_root.is_dir():
            candidates = sorted([item for item in cask_root.iterdir() if item.is_dir()], key=lambda item: item.name)
            for candidate in reversed(candidates):
                direct = candidate / "Contents" / "NDK"
                if direct.exists():
                    return direct
                apps = sorted(candidate.glob("*.app/Contents/NDK"))
                if apps:
                    return apps[-1]
                for fallback in ["ndk", "NDK"]:
                    fallback_path = candidate / fallback
                    if fallback_path.exists():
                        return fallback_path
    return None


def _cmake_version_ok() -> Tuple[bool, str]:
    cmake_bin = shutil.which("../cmake")
    if not cmake_bin:
        return False, "cmake was not found in PATH."

    probe = _run_capture([cmake_bin, "--version"])
    if probe.returncode != 0:
        return False, "cmake exists but version probe failed."

    match = re.search(r"cmake version (\d+)\.(\d+)\.(\d+)", probe.stdout)
    if not match:
        return False, "cmake version could not be parsed."
    major, minor, _ = (int(value) for value in match.groups())
    if (major, minor) < (3, 24):
        return False, f"cmake>={3}.{24} is required, found {match.group(0).split()[-1]}."
    return True, ""


def _build_manual_actions(system_name: str, env_map: Dict[str, str], tools: Dict[str, bool]) -> List[str]:
    actions: List[str] = []

    cmake_ok, cmake_msg = _cmake_version_ok()
    if not cmake_ok:
        actions.append(f"Install/upgrade CMake 3.24+ ({cmake_msg})")

    if not tools["python3"]:
        actions.append("Install Python 3 and ensure python3 is available in PATH.")
    if not tools["git"]:
        actions.append("Install Git and ensure git is available in PATH.")
    if not tools["cargo"]:
        actions.append("Install Rust toolchain (rustup + cargo) for src/cli build target.")

    qt_host_prefix = Path(env_map["QT_HOST_PREFIX"])
    if not qt_host_prefix.exists():
        actions.append(
            f"Install Qt host kit or set QT_HOST_PREFIX. Expected path: {qt_host_prefix}"
        )

    qt_android_prefix = Path(env_map["QT_ANDROID_PREFIX"])
    if not qt_android_prefix.exists():
        actions.append(
            f"Install Qt Android kit or set QT_ANDROID_PREFIX. Expected path: {qt_android_prefix}"
        )

    if system_name == "Darwin":
        qt_ios_prefix = Path(env_map["QT_IOS_PREFIX"])
        if not qt_ios_prefix.exists():
            actions.append(
                f"Install Qt iOS kit or set QT_IOS_PREFIX. Expected path: {qt_ios_prefix}"
            )
        if not tools["xcrun"]:
            actions.append("Install Xcode Command Line Tools (xcode-select --install).")

    lvrs_dir = Path(env_map["LVRS_DIR"])
    if not lvrs_dir.exists():
        actions.append(
            f"Install LVRS package and/or set LVRS_PREFIX. Expected LVRS_DIR path: {lvrs_dir}"
        )

    android_sdk_root = Path(env_map["ANDROID_SDK_ROOT"])
    if not android_sdk_root.exists():
        actions.append(
            f"Install Android SDK or set ANDROID_SDK_ROOT. Expected path: {android_sdk_root}"
        )

    android_ndk_root = env_map.get("ANDROID_NDK_ROOT", "").strip()
    if not android_ndk_root:
        actions.append(
            "Install Android NDK and set ANDROID_NDK_ROOT, or install under ANDROID_SDK_ROOT/ndk."
        )
    elif not Path(android_ndk_root).exists():
        actions.append(
            f"Configured ANDROID_NDK_ROOT does not exist: {android_ndk_root}"
        )

    if not env_map.get("ANDROID_AVD", "").strip():
        actions.append(
            "Create at least one Android AVD (Android Studio Device Manager) or set ANDROID_AVD."
        )

    java_home = env_map.get("JAVA21_HOME", "").strip()
    if java_home and not Path(java_home).exists():
        actions.append(f"Configured JAVA21_HOME does not exist: {java_home}")
    if not java_home:
        actions.append("Install JDK 21 and set JAVA21_HOME for Android builds.")

    return actions


def _write_json(path: Path, payload: Dict[str, object]) -> None:
    path.write_text(json.dumps(payload, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")


def _write_shell(path: Path, env_map: Dict[str, str], generated_at: str) -> None:
    lines = [
        "#!/usr/bin/env bash",
        "set -euo pipefail",
        f"# Generated by dev_env.py at {generated_at}",
        "",
    ]
    for key in sorted(env_map.keys()):
        value = env_map[key]
        if value == "":
            continue
        lines.append(f"export {key}={shlex.quote(value)}")
    lines.append("")
    lines.append('echo "[whatson-dev-env] environment loaded from build/dev-env/dev_env.sh"')
    path.write_text("\n".join(lines) + "\n", encoding="utf-8")
    path.chmod(0o755)


def _write_build_all_wrapper(path: Path, repo_root: Path, env_sh_path: Path) -> None:
    script = textwrap.dedent(
        f"""\
        #!/usr/bin/env bash
        set -euo pipefail
        source {shlex.quote(str(env_sh_path))}
        exec python3 {shlex.quote(str(repo_root / "scripts" / "build_all.py"))} "$@"
        """
    )
    path.write_text(script, encoding="utf-8")
    path.chmod(0o755)


def _write_brief(path: Path, env_sh_path: Path, build_all_wrapper_path: Path, manual_actions: List[str]) -> None:
    lines = [
        "WhatSon Development Environment",
        "================================",
        "",
        f"1) Load environment: source {env_sh_path}",
        f"2) Run unified build: {build_all_wrapper_path} --tasks host,android,ios",
        "",
    ]
    if manual_actions:
        lines.append("Manual actions required:")
        for idx, action in enumerate(manual_actions, start=1):
            lines.append(f"{idx}. {action}")
    else:
        lines.append("Manual actions required: none")
    lines.append("")
    path.write_text("\n".join(lines), encoding="utf-8")


def parse_args() -> argparse.Namespace:
    repo_root = Path(__file__).resolve().parent
    parser = argparse.ArgumentParser(
        description="Normalize WhatSon local development environment for this machine."
    )
    parser.add_argument("--repo-root", default=str(repo_root), help="WhatSon repository root.")
    parser.add_argument(
        "--output-dir",
        default=str(repo_root / "build" / "dev-env"),
        help="Directory for generated environment artifacts.",
    )
    parser.add_argument(
        "--strict",
        action="store_true",
        help="Exit with non-zero status if manual actions are required.",
    )
    parser.add_argument(
        "--print-only",
        action="store_true",
        help="Resolve environment and print summary without writing files.",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    repo_root = _expand(args.repo_root)
    output_dir = _expand(args.output_dir)
    system_name = platform.system()
    home = _expand("~")

    qt_version_root = _expand(
        os.environ.get(
            "QT_VERSION_ROOT",
            str(_default_qt_version_root(home)),
        )
    )
    qt_host_prefix = _expand(
        os.environ.get("QT_HOST_PREFIX", str(_default_qt_host_prefix(system_name, qt_version_root))))
    qt_ios_prefix = _expand(os.environ.get("QT_IOS_PREFIX", str(qt_version_root / "ios")))
    qt_android_prefix = _expand(
        os.environ.get("QT_ANDROID_PREFIX", str(_default_qt_android_prefix(qt_version_root)))
    )

    lvrs_prefix = _resolve_lvrs_prefix(home, repo_root)
    lvrs_dir = _resolve_lvrs_dir(lvrs_prefix, system_name)
    lvrs_android_prefix = _resolve_lvrs_android_prefix(home, lvrs_prefix)
    lvrs_source_dir = _default_lvrs_source_dir(home, repo_root)

    android_sdk_root = _expand(
        os.environ.get("ANDROID_SDK_ROOT", str(_default_android_sdk(system_name, home)))
    )
    android_ndk_root = _resolve_android_ndk(android_sdk_root)
    android_avd = _detect_avd_name()
    java21_home = _resolve_java21_home(system_name)
    android_package = os.environ.get("WHATSON_ANDROID_PACKAGE") or _read_android_manifest_package(
        repo_root) or DEFAULT_ANDROID_PACKAGE_ID
    apple_bundle_id = os.environ.get("WHATSON_APPLE_BUNDLE_ID") or DEFAULT_APPLE_BUNDLE_ID

    env_map: Dict[str, str] = {
        "WHATSON_ROOT": str(repo_root),
        "QT_VERSION_ROOT": str(qt_version_root),
        "QT_HOST_PREFIX": str(qt_host_prefix),
        "QT_IOS_PREFIX": str(qt_ios_prefix),
        "QT_ANDROID_PREFIX": str(qt_android_prefix),
        "LVRS_PREFIX": str(lvrs_prefix),
        "LVRS_DIR": str(lvrs_dir),
        "LVRS_ANDROID_PREFIX": str(lvrs_android_prefix),
        "ANDROID_SDK_ROOT": str(android_sdk_root),
        "ANDROID_HOME": str(android_sdk_root),
        "ANDROID_NDK_ROOT": str(android_ndk_root) if android_ndk_root else "",
        "ANDROID_NDK": str(android_ndk_root) if android_ndk_root else "",
        "CMAKE_ANDROID_NDK": str(android_ndk_root) if android_ndk_root else "",
        "QT_ANDROID_NDK_ROOT": str(android_ndk_root) if android_ndk_root else "",
        "ANDROID_AVD": android_avd or "",
        "WHATSON_ANDROID_PACKAGE": android_package,
        "WHATSON_APPLE_BUNDLE_ID": apple_bundle_id,
        "JAVA21_HOME": str(java21_home) if java21_home else "",
    }
    if lvrs_source_dir:
        env_map["LVRS_SOURCE_DIR"] = str(lvrs_source_dir)

    tools = {
        "python3": shutil.which("python3") is not None,
        "git": shutil.which("git") is not None,
        "cargo": shutil.which("cargo") is not None,
        "xcrun": shutil.which("xcrun") is not None,
    }
    manual_actions = _build_manual_actions(system_name, env_map, tools)

    if not args.print_only:
        output_dir.mkdir(parents=True, exist_ok=True)
        json_path = output_dir / "dev_env.json"
        sh_path = output_dir / "dev_env.sh"
        build_all_wrapper_path = output_dir / "build_all.sh"
        brief_path = output_dir / "README.txt"

        generated_at = subprocess.run(
            ["date", "+%Y-%m-%dT%H:%M:%S%z"], capture_output=True, text=True, check=False
        ).stdout.strip()
        if not generated_at:
            generated_at = "unknown"

        payload: Dict[str, object] = {
            "repo_root": str(repo_root),
            "system": system_name,
            "environment": env_map,
            "tool_check": tools,
            "manual_actions": manual_actions,
        }
        _write_json(json_path, payload)
        _write_shell(sh_path, env_map, generated_at)
        _write_build_all_wrapper(build_all_wrapper_path, repo_root, sh_path)
        _write_brief(brief_path, sh_path, build_all_wrapper_path, manual_actions)

    print(f"[dev_env] repo_root={repo_root}")
    print(f"[dev_env] output_dir={output_dir}")
    print("[dev_env] resolved environment:")
    for key in sorted(env_map.keys()):
        print(f"  - {key}={env_map[key]}")

    if manual_actions:
        print("[dev_env] manual actions required:")
        for idx, action in enumerate(manual_actions, start=1):
            print(f"  {idx}. {action}")
    else:
        print("[dev_env] manual actions required: none")

    if args.strict and manual_actions:
        return 2
    return 0


if __name__ == "__main__":
    sys.exit(main())
