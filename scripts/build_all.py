#!/usr/bin/env python3
from __future__ import annotations

import argparse
import json
import os
import shlex
import shutil
import subprocess
import sys
import time
import xml.etree.ElementTree as ET
from concurrent.futures import ThreadPoolExecutor, as_completed
from dataclasses import dataclass
from pathlib import Path
from typing import Dict, Iterable, List, Optional, Sequence

import platform

TASK_HOST = "host"
TASK_ANDROID = "android"
TASK_IOS = "ios"
ALL_TASKS = (TASK_HOST, TASK_ANDROID, TASK_IOS)


@dataclass
class TaskResult:
    name: str
    status: str  # success, failed, skipped
    detail: str
    log_path: Path


class CommandError(RuntimeError):
    pass


def _expand(path: str) -> Path:
    return Path(os.path.expandvars(os.path.expanduser(path))).resolve()


def _default_android_sdk(system_name: str, home: Path) -> Path:
    if system_name == "Darwin":
        return home / "Library" / "Android" / "sdk"
    if system_name == "Windows":
        localappdata = os.environ.get("LOCALAPPDATA")
        if localappdata:
            return _expand(localappdata) / "Android" / "Sdk"
    return home / "Android" / "Sdk"


def _runtime_platform_name(system_name: str) -> str:
    if system_name == "Darwin":
        return "macos"
    if system_name == "Windows":
        return "windows"
    if system_name == "Linux":
        return "linux"
    return "unknown"


def _lvrs_root_from_prefix(prefix: Path) -> Path:
    if prefix.name in {"macos", "windows", "linux", "ios", "android"} and prefix.parent.name == "platforms":
        return prefix.parent.parent
    return prefix


def _default_lvrs_prefix_for_system(system_name: str, home: Path) -> Path:
    lvrs_root = home / ".local" / "LVRS"
    runtime = _runtime_platform_name(system_name)
    platform_prefix = lvrs_root / "platforms" / runtime
    if platform_prefix.exists():
        return platform_prefix
    return lvrs_root


def _default_lvrs_ios_prefix(base_prefix: Path) -> Path:
    lvrs_root = _lvrs_root_from_prefix(base_prefix)
    platform_prefix = lvrs_root / "platforms" / "ios"
    if platform_prefix.exists():
        return platform_prefix
    return base_prefix


def _default_lvrs_android_prefix(base_prefix: Path, home: Path) -> Path:
    lvrs_root = _lvrs_root_from_prefix(base_prefix)
    platform_prefix = lvrs_root / "platforms" / "android"
    if platform_prefix.exists():
        return platform_prefix
    return home / ".local" / "LVRS-android"


def _android_manifest_candidates(root: Path) -> List[Path]:
    return [
        root / "platform" / "Android" / "AndroidManifest.xml",
        root / "platform" / "android" / "AndroidManifest.xml",
        root / "android" / "AndroidManifest.xml",
    ]


def _read_android_package_from_manifest(root: Path) -> Optional[str]:
    for manifest in _android_manifest_candidates(root):
        if not manifest.exists():
            continue
        try:
            xml_root = ET.parse(manifest).getroot()
        except ET.ParseError:
            continue
        package_name = xml_root.attrib.get("package", "").strip()
        if package_name:
            return package_name
    return None


def _default_qt_host_prefix(system_name: str, qt_root: Path) -> Path:
    if system_name == "Darwin":
        candidates = [qt_root / "macos"]
    elif system_name == "Windows":
        candidates = [
            qt_root / "msvc2022_64",
            qt_root / "msvc2019_64",
            qt_root / "mingw_64",
        ]
    else:
        candidates = [qt_root / "gcc_64"]

    existing = _find_existing(candidates)
    if existing:
        return existing
    return candidates[0]


def _default_qt_android_prefix(qt_root: Path) -> Path:
    candidates = [
        qt_root / "android_arm64_v8a",
        qt_root / "android_x86_64",
        qt_root / "android",
    ]
    existing = _find_existing(candidates)
    if existing:
        return existing
    return candidates[0]


def _latest_dir(parent: Path) -> Optional[Path]:
    if not parent.is_dir():
        return None
    dirs = [d for d in parent.iterdir() if d.is_dir()]
    if not dirs:
        return None
    return sorted(dirs)[-1]


def _find_existing(paths: Iterable[Path]) -> Optional[Path]:
    for path in paths:
        if path.exists():
            return path
    return None


class BuildAll:
    def __init__(self, args: argparse.Namespace) -> None:
        self.args = args
        self.system_name = platform.system()
        self.home = _expand("~")
        self.root = _expand(args.root)
        self.logs_dir = _expand(args.logs_dir)
        self.logs_dir.mkdir(parents=True, exist_ok=True)

        self.qt_version_root = _expand(args.qt_version_root)
        self.qt_host_prefix = _expand(args.qt_host_prefix)
        self.qt_ios_prefix = _expand(args.qt_ios_prefix)
        self.qt_android_prefix = _expand(args.qt_android_prefix)

        self.lvrs_prefix = _expand(args.lvrs_prefix)
        self.ios_lvrs_prefix = _expand(args.ios_lvrs_prefix)
        self.android_lvrs_prefix = _expand(args.android_lvrs_prefix)
        self.lvrs_source_dir = _expand(args.lvrs_source_dir)

        self.host_build_dir = _expand(args.host_build_dir)
        self.android_build_dir = _expand(args.android_build_dir)
        self.ios_project_dir = _expand(args.ios_project_dir)
        self.android_studio_dir = _expand(args.android_studio_dir)

        self.android_sdk_root = _expand(args.android_sdk_root)
        self.android_ndk_root = _expand(args.android_ndk_root) if args.android_ndk_root else None
        if self.android_ndk_root is None:
            detected = _latest_dir(self.android_sdk_root / "ndk")
            if detected:
                self.android_ndk_root = detected
        if self.android_ndk_root is None and self.system_name == "Darwin":
            fallback_ndk = Path("/opt/homebrew/share/android-ndk")
            if fallback_ndk.exists():
                self.android_ndk_root = fallback_ndk

        self.android_package_explicit = bool(args.android_package)
        discovered_android_package = _read_android_package_from_manifest(self.root)
        self.android_package = args.android_package or discovered_android_package or "org.qtproject.example.WhatSon"
        self.android_avd = args.android_avd
        self.skip_android_lvrs_build = args.skip_android_lvrs_build
        self.no_host_run = args.no_host_run
        self.sequential = args.sequential

        self.java21_home = _expand(args.java21_home) if args.java21_home else None

    def _log(self, task: str, message: str) -> None:
        print(f"[{task}] {message}")

    def _quote_cmd(self, cmd: Sequence[str]) -> str:
        return " ".join(shlex.quote(str(item)) for item in cmd)

    def _run(
            self,
            *,
            task: str,
            cmd: Sequence[str],
            cwd: Optional[Path] = None,
            env: Optional[Dict[str, str]] = None,
            log_path: Path,
            check: bool = True,
    ) -> int:
        cwd_path = cwd or self.root
        merged_env = os.environ.copy()
        if env:
            merged_env.update(env)
        log_path.parent.mkdir(parents=True, exist_ok=True)

        with log_path.open("a", encoding="utf-8") as fp:
            fp.write(f"$ {self._quote_cmd(cmd)}\n")
            fp.write(f"# cwd: {cwd_path}\n")
            process = subprocess.run(
                [str(item) for item in cmd],
                cwd=str(cwd_path),
                env=merged_env,
                stdout=fp,
                stderr=subprocess.STDOUT,
                text=True,
            )
            fp.write(f"[exit] {process.returncode}\n\n")

        if check and process.returncode != 0:
            raise CommandError(f"Command failed ({process.returncode}): {self._quote_cmd(cmd)}")
        return process.returncode

    def _capture(
            self,
            *,
            cmd: Sequence[str],
            cwd: Optional[Path] = None,
            env: Optional[Dict[str, str]] = None,
    ) -> subprocess.CompletedProcess[str]:
        merged_env = os.environ.copy()
        if env:
            merged_env.update(env)
        return subprocess.run(
            [str(item) for item in cmd],
            cwd=str(cwd or self.root),
            env=merged_env,
            capture_output=True,
            text=True,
            check=False,
        )

    def _host_app_binary(self) -> Optional[Path]:
        candidates: List[Path] = []
        if self.system_name == "Darwin":
            candidates.append(
                self.host_build_dir / "src" / "app" / "bin" / "WhatSon.app" / "Contents" / "MacOS" / "WhatSon")
        elif self.system_name == "Windows":
            candidates.append(self.host_build_dir / "src" / "app" / "bin" / "WhatSon.exe")
            candidates.append(self.host_build_dir / "src" / "app" / "WhatSon.exe")
        else:
            candidates.append(self.host_build_dir / "src" / "app" / "bin" / "WhatSon")
            candidates.append(self.host_build_dir / "src" / "app" / "WhatSon")
        return _find_existing(candidates)

    def _daemon_binary(self) -> Optional[Path]:
        candidates = [
            self.host_build_dir / "src" / "daemon" / "whats_on_daemon",
            self.host_build_dir / "src" / "daemon" / "whats_on_daemon.exe",
        ]
        return _find_existing(candidates)

    def _find_adb(self) -> Optional[Path]:
        from_path = shutil.which("adb")
        if from_path:
            return _expand(from_path)

        candidates = []
        if self.system_name == "Windows":
            candidates.append(self.android_sdk_root / "platform-tools" / "adb.exe")
        candidates.append(self.android_sdk_root / "platform-tools" / "adb")
        return _find_existing(candidates)

    def _find_emulator_binary(self) -> Optional[Path]:
        from_path = shutil.which("emulator")
        if from_path:
            return _expand(from_path)

        candidates = []
        if self.system_name == "Windows":
            candidates.append(self.android_sdk_root / "emulator" / "emulator.exe")
        candidates.append(self.android_sdk_root / "emulator" / "emulator")
        return _find_existing(candidates)

    def _connected_android_serial(self, adb_path: Path) -> Optional[str]:
        result = self._capture(cmd=[str(adb_path), "devices", "-l"])
        if result.returncode != 0:
            return None
        for line in result.stdout.splitlines():
            parts = line.strip().split()
            if len(parts) >= 2 and parts[1] == "device":
                return parts[0]
        return None

    def _pick_avd(self, emulator_path: Path) -> Optional[str]:
        if self.android_avd:
            return self.android_avd
        result = self._capture(cmd=[str(emulator_path), "-list-avds"])
        if result.returncode != 0:
            return None
        avds = [line.strip() for line in result.stdout.splitlines() if line.strip()]
        if not avds:
            return None
        return avds[0]

    def _ensure_android_device(self, *, task: str, log_path: Path) -> str:
        adb_path = self._find_adb()
        if adb_path is None:
            raise CommandError("adb was not found. Install Android platform-tools or set ANDROID_SDK_ROOT.")

        serial = self._connected_android_serial(adb_path)
        if serial:
            self._log(task, f"Using connected Android device: {serial}")
            return serial

        emulator_path = self._find_emulator_binary()
        if emulator_path is None:
            raise CommandError("Android emulator binary was not found.")

        avd = self._pick_avd(emulator_path)
        if not avd:
            raise CommandError("No Android AVD was found.")

        self._log(task, f"Starting Android emulator AVD: {avd}")
        emu_log = log_path.with_name(f"{log_path.stem}.emulator.log")
        with emu_log.open("a", encoding="utf-8") as fp:
            fp.write(f"$ {self._quote_cmd([str(emulator_path), '-avd', avd])}\n")
            if self.system_name == "Windows":
                flags = 0
                flags |= getattr(subprocess, "DETACHED_PROCESS", 0)
                flags |= getattr(subprocess, "CREATE_NEW_PROCESS_GROUP", 0)
                subprocess.Popen(
                    [str(emulator_path), "-avd", avd, "-netdelay", "none", "-netspeed", "full"],
                    cwd=str(self.root),
                    stdout=fp,
                    stderr=subprocess.STDOUT,
                    creationflags=flags,
                )
            else:
                subprocess.Popen(
                    [str(emulator_path), "-avd", avd, "-netdelay", "none", "-netspeed", "full"],
                    cwd=str(self.root),
                    stdout=fp,
                    stderr=subprocess.STDOUT,
                    start_new_session=True,
                )

        self._run(task=task, cmd=[str(adb_path), "wait-for-device"], log_path=log_path)

        for _ in range(180):
            probe = self._capture(cmd=[str(adb_path), "shell", "getprop", "sys.boot_completed"])
            if probe.returncode == 0 and probe.stdout.strip() == "1":
                serial = self._connected_android_serial(adb_path)
                if serial:
                    self._log(task, f"Android emulator is ready: {serial}")
                    return serial
            time.sleep(2)

        raise CommandError("Android emulator boot timed out.")

    def _prepare_java21_env(self, env: Dict[str, str]) -> Dict[str, str]:
        if self.java21_home and self.java21_home.exists():
            updated = dict(env)
            updated["JAVA_HOME"] = str(self.java21_home)
            java_bin = self.java21_home / "bin"
            updated["PATH"] = f"{java_bin}{os.pathsep}{updated.get('PATH', '')}"
            return updated
        return env

    def _ensure_android_lvrs_prefix(self, *, task: str, log_path: Path) -> Path:
        config = self.android_lvrs_prefix / "lib" / "cmake" / "LVRS" / "LVRSConfig.cmake"
        if config.exists():
            return self.android_lvrs_prefix

        if self.skip_android_lvrs_build:
            raise CommandError(
                f"Android LVRS prefix is missing ({self.android_lvrs_prefix}) and --skip-android-lvrs-build was set."
            )

        if not self.lvrs_source_dir.exists():
            raise CommandError(
                f"Android LVRS prefix is missing and LVRS source dir was not found: {self.lvrs_source_dir}"
            )

        if self.android_ndk_root is None or not self.android_ndk_root.exists():
            raise CommandError("Android NDK was not found. Install NDK and set ANDROID_NDK_ROOT.")

        toolchain = self.qt_android_prefix / "lib" / "cmake" / "Qt6" / "qt.toolchain.cmake"
        if not toolchain.exists():
            raise CommandError(f"Qt Android toolchain file is missing: {toolchain}")

        if not self.android_sdk_root.exists():
            raise CommandError(f"Android SDK root is missing: {self.android_sdk_root}")

        self._log(task, f"Building Android LVRS package at: {self.android_lvrs_prefix}")
        lvrs_build_dir = self.lvrs_source_dir / "build-android-install"
        lvrs_prefix_path = str(self.qt_android_prefix)
        env = os.environ.copy()
        env["ANDROID_SDK_ROOT"] = str(self.android_sdk_root)
        env["ANDROID_HOME"] = str(self.android_sdk_root)
        env["ANDROID_NDK_ROOT"] = str(self.android_ndk_root)
        env["ANDROID_NDK_HOME"] = str(self.android_ndk_root)
        env["CMAKE_ANDROID_NDK"] = str(self.android_ndk_root)

        self._run(
            task=task,
            cmd=[
                "cmake",
                "-S",
                str(self.lvrs_source_dir),
                "-B",
                str(lvrs_build_dir),
                "-DCMAKE_SYSTEM_NAME=Android",
                f"-DCMAKE_TOOLCHAIN_FILE={toolchain}",
                f"-DCMAKE_PREFIX_PATH={lvrs_prefix_path}",
                "-DCMAKE_BUILD_TYPE=Release",
                "-DCMAKE_ANDROID_ARCH_ABI=arm64-v8a",
                f"-DCMAKE_ANDROID_NDK={self.android_ndk_root}",
                f"-DANDROID_NDK_ROOT={self.android_ndk_root}",
                f"-DCMAKE_ANDROID_SDK={self.android_sdk_root}",
                f"-DANDROID_SDK_ROOT={self.android_sdk_root}",
                f"-DANDROID_HOME={self.android_sdk_root}",
                "-DLVRS_BUILD_EXAMPLES=OFF",
                "-DLVRS_BUILD_TESTS=OFF",
                f"-DCMAKE_INSTALL_PREFIX={self.android_lvrs_prefix}",
            ],
            env=env,
            log_path=log_path,
        )
        self._run(task=task, cmd=["cmake", "--build", str(lvrs_build_dir), "-j"], env=env, log_path=log_path)
        self._run(task=task, cmd=["cmake", "--install", str(lvrs_build_dir)], env=env, log_path=log_path)

        if not config.exists():
            raise CommandError(f"Android LVRS install failed: {config} was not generated.")
        return self.android_lvrs_prefix

    def task_host(self) -> TaskResult:
        task = TASK_HOST
        log_path = self.logs_dir / f"{task}.log"
        try:
            self.host_build_dir.mkdir(parents=True, exist_ok=True)

            cmake_cmd = ["cmake", "-S", str(self.root), "-B", str(self.host_build_dir)]
            prefix_paths: List[str] = []
            if self.qt_host_prefix.exists():
                prefix_paths.append(str(self.qt_host_prefix))
            if self.lvrs_prefix.exists():
                prefix_paths.append(str(self.lvrs_prefix))
            if prefix_paths:
                cmake_cmd.append(f"-DCMAKE_PREFIX_PATH={';'.join(prefix_paths)}")
            lvrs_dir = self.lvrs_prefix / "lib" / "cmake" / "LVRS"
            if lvrs_dir.exists():
                cmake_cmd.append(f"-DLVRS_DIR={lvrs_dir}")

            self._run(task=task, cmd=cmake_cmd, log_path=log_path)
            self._run(task=task, cmd=["cmake", "--build", str(self.host_build_dir), "--target", "WhatSon", "-j"],
                      log_path=log_path)
            self._run(
                task=task,
                cmd=["cmake", "--build", str(self.host_build_dir), "--target", "whats_on_daemon", "-j"],
                log_path=log_path,
            )

            daemon = self._daemon_binary()
            if daemon:
                self._run(task=task, cmd=[str(daemon), "--healthcheck"], log_path=log_path)

            if self.no_host_run:
                return TaskResult(task, "success", "Build completed (host run skipped by flag).", log_path)

            app_bin = self._host_app_binary()
            if app_bin is None:
                return TaskResult(task, "failed", "Build succeeded but host app binary was not found.", log_path)

            self._log(task, f"Launching host app: {app_bin}")
            with log_path.open("a", encoding="utf-8") as fp:
                fp.write(f"$ launch {app_bin}\n")
                if self.system_name == "Windows":
                    flags = 0
                    flags |= getattr(subprocess, "DETACHED_PROCESS", 0)
                    flags |= getattr(subprocess, "CREATE_NEW_PROCESS_GROUP", 0)
                    process = subprocess.Popen(
                        [str(app_bin)],
                        cwd=str(self.root),
                        stdout=fp,
                        stderr=subprocess.STDOUT,
                        creationflags=flags,
                    )
                else:
                    process = subprocess.Popen(
                        [str(app_bin)],
                        cwd=str(self.root),
                        stdout=fp,
                        stderr=subprocess.STDOUT,
                        start_new_session=True,
                    )
                fp.write(f"# pid={process.pid}\n")

            return TaskResult(task, "success", f"Host app launched (pid={process.pid}).", log_path)
        except Exception as exc:  # noqa: BLE001
            return TaskResult(task, "failed", str(exc), log_path)

    def task_ios(self) -> TaskResult:
        task = TASK_IOS
        log_path = self.logs_dir / f"{task}.log"
        if self.system_name != "Darwin":
            return TaskResult(task, "skipped", "iOS Xcode project generation is macOS-only.", log_path)

        try:
            toolchain = self.qt_ios_prefix / "lib" / "cmake" / "Qt6" / "qt.toolchain.cmake"
            if not toolchain.exists():
                raise CommandError(f"Qt iOS toolchain file was not found: {toolchain}")

            prefix_paths = [str(self.qt_ios_prefix), str(self.ios_lvrs_prefix)]
            if self.qt_host_prefix.exists():
                prefix_paths.append(str(self.qt_host_prefix))

            ios_cmd = [
                "cmake",
                "-G",
                "Xcode",
                "-S",
                str(self.root),
                "-B",
                str(self.ios_project_dir),
                "-DCMAKE_SYSTEM_NAME=iOS",
                f"-DCMAKE_TOOLCHAIN_FILE={toolchain}",
                f"-DCMAKE_PREFIX_PATH={';'.join(prefix_paths)}",
                "-DCMAKE_BUILD_TYPE=Release",
                "-DCMAKE_OSX_SYSROOT=iphonesimulator",
                "-DCMAKE_OSX_ARCHITECTURES=arm64",
                "-DCMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_ALLOWED=NO",
                "-DCMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED=NO",
                "-DCMAKE_XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER=com.lvrs.whatson",
            ]

            lvrs_dir = self.ios_lvrs_prefix / "lib" / "cmake" / "LVRS"
            if lvrs_dir.exists():
                ios_cmd.append(f"-DLVRS_DIR={lvrs_dir}")

            self._run(task=task, cmd=ios_cmd, log_path=log_path)

            xcode_project = self.ios_project_dir / "WhatSon.xcodeproj"
            if not xcode_project.exists():
                raise CommandError(f"Xcode project was not generated: {xcode_project}")

            return TaskResult(task, "success", f"Generated: {xcode_project}", log_path)
        except Exception as exc:  # noqa: BLE001
            return TaskResult(task, "failed", str(exc), log_path)

    def task_android(self) -> TaskResult:
        task = TASK_ANDROID
        log_path = self.logs_dir / f"{task}.log"
        try:
            adb_path = self._find_adb()
            if adb_path is None:
                raise CommandError("adb was not found.")

            serial = self._ensure_android_device(task=task, log_path=log_path)
            _ = serial

            if self.system_name == "Darwin":
                result = self._task_android_macos_proven(task=task, log_path=log_path, adb_path=adb_path)
                if result:
                    return TaskResult(task, "success", result, log_path)
            else:
                result = self._task_android_generic(task=task, log_path=log_path)
                if result:
                    return TaskResult(task, "success", result, log_path)

            return TaskResult(task, "failed", "Android launch did not complete.", log_path)
        except Exception as exc:  # noqa: BLE001
            return TaskResult(task, "failed", str(exc), log_path)

    def _task_android_generic(self, *, task: str, log_path: Path) -> str:
        self.android_build_dir.mkdir(parents=True, exist_ok=True)
        cmake_cmd = ["cmake", "-S", str(self.root), "-B", str(self.android_build_dir)]

        prefix_paths: List[str] = []
        if self.qt_host_prefix.exists():
            prefix_paths.append(str(self.qt_host_prefix))
        if self.lvrs_prefix.exists():
            prefix_paths.append(str(self.lvrs_prefix))
        if prefix_paths:
            cmake_cmd.append(f"-DCMAKE_PREFIX_PATH={';'.join(prefix_paths)}")
        lvrs_dir = self.lvrs_prefix / "lib" / "cmake" / "LVRS"
        if lvrs_dir.exists():
            cmake_cmd.append(f"-DLVRS_DIR={lvrs_dir}")

        self._run(task=task, cmd=cmake_cmd, log_path=log_path)
        self._run(task=task,
                  cmd=["cmake", "--build", str(self.android_build_dir), "--target", "launch_WhatSon_android"],
                  log_path=log_path)
        gradle_dir = self._find_android_gradle_dir()
        if gradle_dir is None:
            return "launch_WhatSon_android completed."
        studio_dir = self._export_android_studio_project(source_dir=gradle_dir, task=task, log_path=log_path)
        return f"launch_WhatSon_android completed. Android Studio project: {studio_dir}"

    def _task_android_macos_proven(self, *, task: str, log_path: Path, adb_path: Path) -> Optional[str]:
        if not self.qt_android_prefix.exists():
            raise CommandError(f"Qt Android prefix was not found: {self.qt_android_prefix}")
        if not self.qt_host_prefix.exists():
            raise CommandError(f"Qt host prefix was not found: {self.qt_host_prefix}")
        if not self.android_sdk_root.exists():
            raise CommandError(f"Android SDK root was not found: {self.android_sdk_root}")
        if self.android_ndk_root is None or not self.android_ndk_root.exists():
            raise CommandError("Android NDK root was not found.")

        android_lvrs = self._ensure_android_lvrs_prefix(task=task, log_path=log_path)
        toolchain = self.qt_android_prefix / "lib" / "cmake" / "Qt6" / "qt.toolchain.cmake"
        if not toolchain.exists():
            raise CommandError(f"Qt Android toolchain file was not found: {toolchain}")

        self.android_build_dir.mkdir(parents=True, exist_ok=True)
        prefix_paths = [str(self.qt_android_prefix), str(android_lvrs), str(self.qt_host_prefix)]

        env = os.environ.copy()
        env["ANDROID_SDK_ROOT"] = str(self.android_sdk_root)
        env["ANDROID_HOME"] = str(self.android_sdk_root)
        env["ANDROID_NDK_ROOT"] = str(self.android_ndk_root)
        env["ANDROID_NDK_HOME"] = str(self.android_ndk_root)
        env["CMAKE_ANDROID_NDK"] = str(self.android_ndk_root)
        env = self._prepare_java21_env(env)

        self._run(
            task=task,
            cmd=[
                "cmake",
                "-S",
                str(self.root),
                "-B",
                str(self.android_build_dir),
                "-DCMAKE_SYSTEM_NAME=Android",
                f"-DCMAKE_TOOLCHAIN_FILE={toolchain}",
                f"-DCMAKE_PREFIX_PATH={';'.join(prefix_paths)}",
                "-DCMAKE_BUILD_TYPE=Release",
                "-DCMAKE_ANDROID_ARCH_ABI=arm64-v8a",
                f"-DCMAKE_ANDROID_NDK={self.android_ndk_root}",
                f"-DANDROID_NDK_ROOT={self.android_ndk_root}",
                f"-DCMAKE_ANDROID_SDK={self.android_sdk_root}",
                f"-DANDROID_SDK_ROOT={self.android_sdk_root}",
                f"-DANDROID_HOME={self.android_sdk_root}",
                f"-DLVRS_DIR={android_lvrs}",
            ],
            env=env,
            log_path=log_path,
        )

        self._run(
            task=task,
            cmd=["cmake", "--build", str(self.android_build_dir), "--target", "WhatSon_make_apk", "-j"],
            env=env,
            log_path=log_path,
        )

        gradle_dir = self._find_android_gradle_dir()
        if gradle_dir is None:
            raise CommandError("Android Gradle project was not generated or gradlew was not found.")

        gradlew = gradle_dir / "gradlew"
        gradlew_bat = gradle_dir / "gradlew.bat"
        if gradlew.exists():
            if self.system_name != "Windows":
                gradlew.chmod(0o755)
            gradle_cmd = [str(gradlew), "assembleDebug"]
        elif gradlew_bat.exists():
            gradle_cmd = [str(gradlew_bat), "assembleDebug"]
        else:
            raise CommandError("gradlew script was not found in android-build output.")

        self._run(task=task, cmd=gradle_cmd, cwd=gradle_dir, env=env, log_path=log_path)

        debug_output_dir = gradle_dir / "build" / "outputs" / "apk" / "debug"
        debug_apks = sorted(debug_output_dir.glob("*debug*.apk")) if debug_output_dir.exists() else []
        if not debug_apks:
            debug_apks = sorted((gradle_dir / "build" / "outputs" / "apk").glob("**/*debug*.apk"))
        if not debug_apks:
            raise CommandError("Debug APK was not generated.")
        debug_apk = debug_apks[-1]

        launch_package = self.android_package
        if not self.android_package_explicit:
            discovered_runtime_package = self._detect_android_runtime_package(gradle_dir)
            if discovered_runtime_package:
                launch_package = discovered_runtime_package

        self._run(task=task, cmd=[str(adb_path), "install", "-r", str(debug_apk)], log_path=log_path)
        self._run(
            task=task,
            cmd=[str(adb_path), "shell", "monkey", "-p", launch_package, "-c", "android.intent.category.LAUNCHER", "1"],
            log_path=log_path,
        )
        studio_dir = self._export_android_studio_project(source_dir=gradle_dir, task=task, log_path=log_path)

        pid_probe = self._capture(cmd=[str(adb_path), "shell", "pidof", launch_package])
        pid_text = pid_probe.stdout.strip() if pid_probe.returncode == 0 else ""
        if pid_text:
            return (
                f"Installed and launched on Android emulator ({launch_package}, pid={pid_text}). "
                f"Android Studio project: {studio_dir}"
            )
        return f"Installed and launched on Android emulator ({launch_package}). Android Studio project: {studio_dir}"

    def _find_android_gradle_dir(self) -> Optional[Path]:
        candidate_dirs = [
            self.android_build_dir / "src" / "app" / "android-build",
            self.android_build_dir / "android-build",
            self.android_build_dir / "src" / "app",
        ]
        for path in candidate_dirs:
            if (path / "gradlew").exists() or (path / "gradlew.bat").exists():
                return path

        # Fallback scan for refactored build layouts.
        for gradlew in self.android_build_dir.glob("**/gradlew"):
            return gradlew.parent
        for gradlew_bat in self.android_build_dir.glob("**/gradlew.bat"):
            return gradlew_bat.parent
        return None

    def _export_android_studio_project(self, *, source_dir: Path, task: str, log_path: Path) -> Path:
        if not source_dir.exists():
            raise CommandError(f"Android Gradle project does not exist: {source_dir}")

        target_dir = self.android_studio_dir
        target_dir.parent.mkdir(parents=True, exist_ok=True)
        if target_dir.exists():
            shutil.rmtree(target_dir)

        ignore = shutil.ignore_patterns(
            ".gradle",
            "build",
            "*.apk",
            "*.aab",
            "*.iml",
            ".idea",
        )
        shutil.copytree(source_dir, target_dir, ignore=ignore, dirs_exist_ok=False)

        gradlew = target_dir / "gradlew"
        if gradlew.exists() and self.system_name != "Windows":
            gradlew.chmod(0o755)

        with log_path.open("a", encoding="utf-8") as fp:
            fp.write(f"# exported android studio project: {target_dir}\n")
            fp.write(f"# source: {source_dir}\n")
        return target_dir

    def _detect_android_runtime_package(self, gradle_dir: Path) -> Optional[str]:
        metadata_candidates = [
            gradle_dir / "build" / "outputs" / "apk" / "debug" / "output-metadata.json",
            gradle_dir / "build" / "outputs" / "apk" / "release" / "output-metadata.json",
        ]
        for metadata_path in metadata_candidates:
            if not metadata_path.exists():
                continue
            try:
                metadata = json.loads(metadata_path.read_text(encoding="utf-8"))
            except Exception:  # noqa: BLE001
                continue
            package_name = metadata.get("applicationId")
            if isinstance(package_name, str) and package_name.strip():
                return package_name.strip()
            elements = metadata.get("elements")
            if isinstance(elements, list):
                for element in elements:
                    if not isinstance(element, dict):
                        continue
                    package_name = element.get("applicationId")
                    if isinstance(package_name, str) and package_name.strip():
                        return package_name.strip()

        gradle_properties = gradle_dir / "gradle.properties"
        if gradle_properties.exists():
            for line in gradle_properties.read_text(encoding="utf-8").splitlines():
                candidate = line.strip()
                if candidate.startswith("androidPackageName="):
                    package_name = candidate.split("=", 1)[1].strip()
                    if package_name:
                        return package_name

        manifest = gradle_dir / "AndroidManifest.xml"
        if manifest.exists():
            try:
                xml_root = ET.parse(manifest).getroot()
                package_name = xml_root.attrib.get("package", "").strip()
                if package_name:
                    return package_name
            except ET.ParseError:
                pass
        return None

    def run(self, tasks: Sequence[str]) -> List[TaskResult]:
        selected = [task for task in tasks if task in ALL_TASKS]
        if not selected:
            raise CommandError("No valid tasks were selected.")

        task_map = {
            TASK_HOST: self.task_host,
            TASK_ANDROID: self.task_android,
            TASK_IOS: self.task_ios,
        }

        if self.sequential:
            return [task_map[name]() for name in selected]

        results: Dict[str, TaskResult] = {}
        with ThreadPoolExecutor(max_workers=len(selected)) as executor:
            future_map = {executor.submit(task_map[name]): name for name in selected}
            for future in as_completed(future_map):
                name = future_map[future]
                try:
                    results[name] = future.result()
                except Exception as exc:  # noqa: BLE001
                    results[name] = TaskResult(name, "failed", str(exc), self.logs_dir / f"{name}.log")

        ordered = [results[name] for name in selected]
        return ordered


def parse_args() -> argparse.Namespace:
    home = _expand("~")
    system_name = platform.system()
    qt_root = _expand(os.environ.get("QT_VERSION_ROOT", str(home / "Qt" / "6.8.3")))
    qt_host_default = _default_qt_host_prefix(system_name, qt_root)
    qt_android_default = _default_qt_android_prefix(qt_root)
    android_sdk = _expand(os.environ.get("ANDROID_SDK_ROOT", str(_default_android_sdk(system_name, home))))
    lvrs_host_default = _default_lvrs_prefix_for_system(system_name, home)
    lvrs_prefix_env = os.environ.get("LVRS_PREFIX")
    lvrs_base_prefix = _expand(lvrs_prefix_env) if lvrs_prefix_env else lvrs_host_default
    lvrs_ios_default = _default_lvrs_ios_prefix(lvrs_base_prefix)
    lvrs_android_default = _default_lvrs_android_prefix(lvrs_base_prefix, home)

    parser = argparse.ArgumentParser(
        description=(
            "Build and run WhatSon on the current host, launch on Android emulator, "
            "and generate iOS Xcode project in one automation entrypoint."
        )
    )
    parser.add_argument("--root", default=str(Path(__file__).resolve().parents[1]), help="Repository root path.")
    parser.add_argument("--logs-dir", default=str(Path(__file__).resolve().parents[1] / "build" / "automation-logs"))
    parser.add_argument("--tasks", default="host,android,ios", help="Comma-separated tasks: host,android,ios")
    parser.add_argument("--sequential", action="store_true", help="Run tasks sequentially instead of parallel.")
    parser.add_argument("--no-host-run", action="store_true", help="Skip launching the host desktop app.")

    parser.add_argument("--host-build-dir", default=str(Path(__file__).resolve().parents[1] / "build" / "host-auto"))
    parser.add_argument("--android-build-dir",
                        default=str(Path(__file__).resolve().parents[1] / "build" / "android-auto"))
    parser.add_argument("--ios-project-dir",
                        default=str(Path(__file__).resolve().parents[1] / "build" / "ios-xcode-artifact"))
    parser.add_argument(
        "--android-studio-dir",
        default=str(Path(__file__).resolve().parents[1] / "build" / "android-studio-artifact"),
        help="Exported Android Studio project path.",
    )

    parser.add_argument("--qt-version-root", default=str(qt_root))
    parser.add_argument("--qt-host-prefix",
                        default=str(_expand(os.environ.get("QT_HOST_PREFIX", str(qt_host_default)))))
    parser.add_argument("--qt-ios-prefix", default=str(_expand(os.environ.get("QT_IOS_PREFIX", str(qt_root / "ios")))))
    parser.add_argument(
        "--qt-android-prefix",
        default=str(_expand(os.environ.get("QT_ANDROID_PREFIX", str(qt_android_default)))),
    )

    parser.add_argument(
        "--lvrs-prefix",
        default=str(_expand(os.environ.get("LVRS_PREFIX", str(lvrs_host_default)))),
        help="LVRS prefix for host/runtime build (supports ~/.local/LVRS/platforms/<runtime> layout).",
    )
    parser.add_argument(
        "--ios-lvrs-prefix",
        default=str(_expand(os.environ.get("LVRS_IOS_PREFIX", str(lvrs_ios_default)))),
        help="LVRS prefix for iOS configure.",
    )
    parser.add_argument(
        "--android-lvrs-prefix",
        default=str(_expand(os.environ.get("LVRS_ANDROID_PREFIX", str(lvrs_android_default)))),
        help="LVRS prefix for Android configure.",
    )
    parser.add_argument("--lvrs-source-dir",
                        default=str(_expand(os.environ.get("LVRS_SOURCE_DIR", str(home / "Developer" / "LVRS")))))
    parser.add_argument("--skip-android-lvrs-build", action="store_true")

    parser.add_argument("--android-sdk-root", default=str(android_sdk))
    parser.add_argument("--android-ndk-root", default=os.environ.get("ANDROID_NDK_ROOT"))
    parser.add_argument("--android-avd", default=os.environ.get("ANDROID_AVD"))
    parser.add_argument(
        "--android-package",
        default=os.environ.get("WHATSON_ANDROID_PACKAGE"),
        help="Android application id/package. Auto-detected from platform AndroidManifest when omitted.",
    )
    parser.add_argument(
        "--java21-home",
        default=os.environ.get(
            "JAVA21_HOME",
            "/opt/homebrew/opt/openjdk@21/libexec/openjdk.jdk/Contents/Home" if system_name == "Darwin" else None,
        ),
    )

    return parser.parse_args()


def main() -> int:
    args = parse_args()
    tasks = [item.strip() for item in args.tasks.split(",") if item.strip()]
    runner = BuildAll(args)

    print(f"[build_all] root={runner.root}")
    print(f"[build_all] tasks={','.join(tasks)}")
    print(f"[build_all] mode={'sequential' if runner.sequential else 'parallel'}")

    try:
        results = runner.run(tasks)
    except Exception as exc:  # noqa: BLE001
        print(f"[build_all] fatal: {exc}")
        return 1

    has_failure = False
    for result in results:
        print(f"[{result.name}] {result.status}: {result.detail}")
        print(f"[{result.name}] log: {result.log_path}")
        if result.status == "failed":
            has_failure = True

    return 1 if has_failure else 0


if __name__ == "__main__":
    sys.exit(main())
