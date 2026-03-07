#!/usr/bin/env python3
from __future__ import annotations
#
# Internal runner module extracted from build_all.py.
# Platform entry scripts (build_host.py, build_android.py, build_ios.py)
# import and execute this logic with a single fixed task.

import argparse
import json
import os
import platform
import plistlib
import re
import shlex
import shutil
import subprocess
import xml.etree.ElementTree as ET
from dataclasses import dataclass
from typing import Dict, Iterable, List, Optional, Sequence, Tuple

import sys
import time
from concurrent.futures import ThreadPoolExecutor, as_completed
from pathlib import Path

TASK_HOST = "host"
TASK_ANDROID = "android"
TASK_IOS = "ios"
ALL_TASKS = (TASK_HOST, TASK_ANDROID, TASK_IOS)
DEFAULT_ANDROID_PACKAGE_ID = "com.iisacc.app.whatson"
DEFAULT_APPLE_BUNDLE_ID = "com.iisacc.app.whatson"
DEFAULT_BUILD_JOBS_CAP = 8
STATE_TEXT_PREVIEW_LIMIT = 240
STATE_ENV_PREFIXES = ("ANDROID", "CMAKE_", "JAVA", "LVRS", "QT", "WHATSON")


@dataclass
class TaskResult:
    name: str
    status: str  # success, failed, skipped
    detail: str
    log_path: Path


class CommandError(RuntimeError):
    pass


class MissingPhysicalDeviceError(CommandError):
    pass


def _expand(path: str) -> Path:
    return Path(os.path.expandvars(os.path.expanduser(path))).resolve()


def _timestamp_now() -> str:
    return time.strftime("%Y-%m-%dT%H:%M:%S%z", time.localtime())


def _preview_text(value: Optional[str], limit: int = STATE_TEXT_PREVIEW_LIMIT) -> Optional[str]:
    if value is None:
        return None
    normalized = value.replace("\r\n", "\n").replace("\r", "\n")
    if len(normalized) <= limit:
        return normalized
    remaining = len(normalized) - limit
    return f"{normalized[:limit]}...(truncated,{remaining} more chars)"


def _state_value(value):  # noqa: ANN001
    if isinstance(value, Path):
        return str(value)
    if isinstance(value, dict):
        return {str(key): _state_value(item) for key, item in value.items()}
    if isinstance(value, (list, tuple, set)):
        return [_state_value(item) for item in value]
    if value is None or isinstance(value, (str, int, float, bool)):
        return value
    return str(value)


def _path_state(path: Optional[Path | str]) -> Optional[Dict[str, object]]:
    if path is None:
        return None
    candidate = path if isinstance(path, Path) else Path(str(path))
    return {
        "path": str(candidate),
        "exists": candidate.exists(),
        "is_dir": candidate.is_dir(),
        "is_file": candidate.is_file(),
    }


def _env_debug_overrides(env: Optional[Dict[str, str]]) -> Dict[str, str]:
    if not env:
        return {}

    overrides: Dict[str, str] = {}
    for key, value in env.items():
        if os.environ.get(key) == value:
            continue
        if key == "PATH":
            preview = _preview_text(value, limit=320)
            if preview is not None:
                overrides[key] = preview
            continue
        if key.startswith(STATE_ENV_PREFIXES):
            overrides[key] = value
    return overrides


def emit_state(script: str, event: str, **fields) -> None:
    payload = {
        "ts": _timestamp_now(),
        "pid": os.getpid(),
        "cwd": str(Path.cwd()),
        "script": script,
        "event": event,
    }
    for key, value in fields.items():
        if value is None:
            continue
        payload[key] = _state_value(value)
    print(f"[state] {json.dumps(payload, ensure_ascii=False, sort_keys=True)}", flush=True)


def _positive_int_arg(value: str) -> int:
    try:
        parsed = int(value)
    except ValueError as exc:
        raise argparse.ArgumentTypeError(f"Expected a positive integer, got '{value}'.") from exc
    if parsed < 1:
        raise argparse.ArgumentTypeError(f"Expected a positive integer, got '{value}'.")
    return parsed


def _default_build_jobs() -> int:
    cpu_count = os.cpu_count() or 1
    return max(1, min(cpu_count, DEFAULT_BUILD_JOBS_CAP))


def _parallel_worker_count(task_count: int, total_jobs: int) -> int:
    if task_count < 1:
        return 0
    return max(1, min(task_count, total_jobs))


def _distribute_job_budget(total_jobs: int, slots: int) -> List[int]:
    if slots < 1:
        return []

    base = total_jobs // slots
    remainder = total_jobs % slots
    budgets = [base] * slots
    for index in range(remainder):
        budgets[index] += 1
    return [max(1, budget) for budget in budgets]


def _task_job_limits(tasks: Sequence[str], total_jobs: int) -> Dict[str, int]:
    worker_count = _parallel_worker_count(len(tasks), total_jobs)
    budgets = _distribute_job_budget(total_jobs, worker_count)
    return {
        task: budgets[index % worker_count]
        for index, task in enumerate(tasks)
    }


def _default_android_sdk(system_name: str, home: Path) -> Path:
    if system_name == "Darwin":
        return home / "Library" / "Android" / "sdk"
    if system_name == "Windows":
        localappdata = os.environ.get("LOCALAPPDATA")
        if localappdata:
            return _expand(localappdata) / "Android" / "Sdk"
    return home / "Android" / "Sdk"


def _default_lvrs_prefix_for_system(system_name: str, home: Path) -> Path:
    _ = system_name
    candidates = [
        home / ".local" / "LVRS",
        Path("/local/LVRS"),
    ]
    return _find_existing(candidates) or candidates[0]


def _default_lvrs_android_prefix(base_prefix: Path, home: Path) -> Path:
    lvrs_root = base_prefix
    if lvrs_root.name in {"macos", "windows", "linux", "ios", "android"} and lvrs_root.parent.name == "platforms":
        lvrs_root = lvrs_root.parent.parent
    platform_prefix = lvrs_root / "platforms" / "android"
    if platform_prefix.exists():
        return platform_prefix
    return home / ".local" / "LVRS-android"


def _default_lvrs_source_dir(home: Path, repo_root: Path) -> Path:
    candidates = [
        home / "Developer" / "LVRS",
        repo_root.parent / "LVRS",
        repo_root.parent.parent / "LVRS",
        repo_root.parent.parent / "InfraSystem" / "LVRS",
        Path("/local/LVRS"),
    ]
    for candidate in candidates:
        if candidate.exists() and (candidate / "CMakeLists.txt").exists():
            return candidate
    return home / "Developer" / "LVRS"


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


def _latest_version_dir(parent: Path, pattern: str = r"^\d+(\.\d+)*$") -> Optional[Path]:
    if not parent.is_dir():
        return None

    regex = re.compile(pattern)
    candidates = [item for item in parent.iterdir() if item.is_dir() and regex.match(item.name)]
    if not candidates:
        return None

    def _key(path: Path) -> Tuple[int, ...]:
        return tuple(int(part) for part in re.findall(r"\d+", path.name))

    return sorted(candidates, key=_key)[-1]


def _default_qt_version_root(home: Path) -> Path:
    qt_home = home / "Qt"
    return _latest_version_dir(qt_home) or qt_home


def _normalize_env_value(value: Optional[str]) -> Optional[str]:
    if value is None:
        return None
    stripped = value.strip()
    return stripped if stripped else None


def _pick_setting(name: str, profile: Dict[str, str], fallback: Optional[str] = None) -> Optional[str]:
    env_value = _normalize_env_value(os.environ.get(name))
    if env_value is not None:
        return env_value
    profile_value = _normalize_env_value(profile.get(name))
    if profile_value is not None:
        return profile_value
    return fallback


def _ndk_from_homebrew_cask(system_name: str) -> Optional[Path]:
    if system_name != "Darwin":
        return None
    cask_root = Path("/opt/homebrew/Caskroom/android-ndk")
    if not cask_root.is_dir():
        return None

    versions = sorted([item for item in cask_root.iterdir() if item.is_dir()], key=lambda item: item.name)
    for version_dir in reversed(versions):
        direct_candidate = version_dir / "Contents" / "NDK"
        if direct_candidate.exists():
            return direct_candidate

        app_candidates = sorted(version_dir.glob("*.app/Contents/NDK"))
        if app_candidates:
            return app_candidates[-1]

        for fallback in ["ndk", "NDK"]:
            fallback_candidate = version_dir / fallback
            if fallback_candidate.exists():
                return fallback_candidate
    return None


def _detect_android_ndk_root(android_sdk_root: Path, system_name: str) -> Optional[Path]:
    explicit = _normalize_env_value(os.environ.get("ANDROID_NDK_ROOT"))
    if explicit:
        return _expand(explicit)

    sdk_ndk = _latest_dir(android_sdk_root / "ndk")
    if sdk_ndk:
        return sdk_ndk

    return _ndk_from_homebrew_cask(system_name)


def _java_major_version(java_home: Path) -> Optional[int]:
    java_bins = [java_home / "bin" / "java"]
    if platform.system() == "Windows":
        java_bins.insert(0, java_home / "bin" / "java.exe")

    java_bin = _find_existing(java_bins)
    if java_bin is None:
        return None

    probe = subprocess.run(
        [str(java_bin), "-version"],
        capture_output=True,
        text=True,
        check=False,
    )
    if probe.returncode != 0:
        return None

    output = f"{probe.stdout}\n{probe.stderr}"
    match = re.search(r'version "([^"]+)"', output)
    if not match:
        return None

    version_text = match.group(1).strip()
    if not version_text:
        return None

    if version_text.startswith("1."):
        major_token = version_text.split(".")[1]
    else:
        major_token = version_text.split(".")[0]
    try:
        return int(major_token)
    except ValueError:
        return None


def _detect_java21_home(system_name: str) -> Optional[Path]:
    explicit = _normalize_env_value(os.environ.get("JAVA21_HOME"))
    if explicit:
        candidate = _expand(explicit)
        if _java_major_version(candidate) == 21:
            return candidate

    if system_name == "Darwin":
        java_home_cmd = Path("/usr/libexec/java_home")
        if java_home_cmd.exists():
            probe = subprocess.run(
                [str(java_home_cmd), "-v", "21"],
                capture_output=True,
                text=True,
                check=False,
            )
            if probe.returncode == 0:
                resolved = probe.stdout.strip()
                if resolved:
                    resolved_path = Path(resolved)
                    if resolved_path.exists() and _java_major_version(resolved_path) == 21:
                        return resolved_path

    candidates = [
        Path("/opt/homebrew/opt/openjdk@21/libexec/openjdk.jdk/Contents/Home"),
        Path("/Library/Java/JavaVirtualMachines/openjdk-21.jdk/Contents/Home"),
        Path("/Library/Java/JavaVirtualMachines/temurin-21.jdk/Contents/Home"),
    ]
    for candidate in candidates:
        if candidate.exists() and _java_major_version(candidate) == 21:
            return candidate

    java_home = _normalize_env_value(os.environ.get("JAVA_HOME"))
    if java_home:
        candidate = _expand(java_home)
        if _java_major_version(candidate) == 21:
            return candidate
    return None


def _load_dev_env_profile(dev_env_json: Path) -> Dict[str, str]:
    if not dev_env_json.exists():
        return {}
    try:
        payload = json.loads(dev_env_json.read_text(encoding="utf-8"))
    except Exception:  # noqa: BLE001
        return {}
    env_map = payload.get("environment")
    if not isinstance(env_map, dict):
        return {}

    normalized: Dict[str, str] = {}
    for key, value in env_map.items():
        if not isinstance(key, str):
            continue
        if value is None:
            continue
        normalized[key] = str(value)
    return normalized


def _resolve_qt_toolchain_file(prefix: Path) -> Path:
    candidates = [
        prefix / "blueprint" / "cmake" / "Qt6" / "qt.toolchain.cmake",
        prefix / "lib" / "cmake" / "Qt6" / "qt.toolchain.cmake",
    ]
    return _find_existing(candidates) or candidates[0]


def _resolve_package_cmake_dir(prefix: Path, package_name: str) -> Path:
    candidates = [
        prefix / "blueprint" / "cmake" / package_name,
        prefix / "lib" / "cmake" / package_name,
    ]
    return _find_existing(candidates) or candidates[0]


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
        self.lvrs_dir = _expand(args.lvrs_dir) if args.lvrs_dir else None
        self.android_lvrs_prefix = _expand(args.android_lvrs_prefix)
        self.lvrs_source_dir = _expand(args.lvrs_source_dir)

        self.host_build_dir = _expand(args.host_build_dir)
        self.android_build_dir = _expand(args.android_build_dir)
        self.ios_project_dir = _expand(args.ios_project_dir)
        self.android_studio_dir = _expand(args.android_studio_dir)

        self.android_sdk_root = _expand(args.android_sdk_root)
        self.android_ndk_root = _expand(args.android_ndk_root) if args.android_ndk_root else None
        if self.android_ndk_root is None:
            self.android_ndk_root = _detect_android_ndk_root(
                android_sdk_root=self.android_sdk_root,
                system_name=self.system_name,
            )

        self.android_package_explicit = bool(args.android_package)
        discovered_android_package = _read_android_package_from_manifest(self.root)
        self.android_package = args.android_package or discovered_android_package or DEFAULT_ANDROID_PACKAGE_ID
        self.ios_bundle_id = args.ios_bundle_id
        self.ios_device = args.ios_device
        self.ios_development_team = args.ios_development_team
        self.ios_code_sign_identity = args.ios_code_sign_identity
        self.android_avd = args.android_avd
        self.android_allow_emulator = args.android_allow_emulator
        self.skip_android_lvrs_build = args.skip_android_lvrs_build
        self.no_host_run = args.no_host_run
        self.sequential = args.sequential
        self.build_jobs = args.jobs
        self._task_job_limit_map: Dict[str, int] = {}

        self.java21_home = _expand(args.java21_home) if args.java21_home else None

    def _emit_state(self, event: str, **fields) -> None:
        emit_state("build_platform_runner", event, **fields)

    def _log(self, task: str, message: str) -> None:
        print(f"[{task}] {message}", flush=True)

    def _quote_cmd(self, cmd: Sequence[str]) -> str:
        return " ".join(shlex.quote(str(item)) for item in cmd)

    def _task_job_limit(self, task: str) -> int:
        return max(1, self._task_job_limit_map.get(task, self.build_jobs))

    def _build_parallel_args(self, task: str) -> List[str]:
        return ["--parallel", str(self._task_job_limit(task))]

    def _xcodebuild_job_args(self, task: str) -> List[str]:
        return ["-jobs", str(self._task_job_limit(task))]

    def _gradle_job_args(self, task: str) -> List[str]:
        return [
            "--max-workers",
            str(self._task_job_limit(task)),
            "-Dorg.gradle.parallel=false",
        ]

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
        cmd_list = [str(item) for item in cmd]
        cmd_text = self._quote_cmd(cmd)
        env_overrides = _env_debug_overrides(merged_env)
        started_at = time.monotonic()

        self._emit_state(
            "command_start",
            task=task,
            cmd=cmd_list,
            cmd_text=cmd_text,
            cwd=_path_state(cwd_path),
            log_path=_path_state(log_path),
            env_overrides=env_overrides or None,
        )

        with log_path.open("a", encoding="utf-8") as fp:
            fp.write(f"$ {cmd_text}\n")
            fp.write(f"# cwd: {cwd_path}\n")
            process = subprocess.run(
                cmd_list,
                cwd=str(cwd_path),
                env=merged_env,
                stdout=fp,
                stderr=subprocess.STDOUT,
                text=True,
            )
            fp.write(f"[exit] {process.returncode}\n\n")

        duration_seconds = round(time.monotonic() - started_at, 3)
        self._emit_state(
            "command_finish",
            task=task,
            cmd=cmd_list,
            cmd_text=cmd_text,
            returncode=process.returncode,
            duration_seconds=duration_seconds,
            log_path=_path_state(log_path),
        )

        if check and process.returncode != 0:
            raise CommandError(f"Command failed ({process.returncode}): {cmd_text}")
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
        cmd_list = [str(item) for item in cmd]
        cmd_text = self._quote_cmd(cmd)
        started_at = time.monotonic()
        self._emit_state(
            "capture_start",
            cmd=cmd_list,
            cmd_text=cmd_text,
            cwd=_path_state(cwd or self.root),
            env_overrides=_env_debug_overrides(merged_env) or None,
        )
        process = subprocess.run(
            cmd_list,
            cwd=str(cwd or self.root),
            env=merged_env,
            capture_output=True,
            text=True,
            check=False,
        )
        self._emit_state(
            "capture_finish",
            cmd=cmd_list,
            cmd_text=cmd_text,
            returncode=process.returncode,
            duration_seconds=round(time.monotonic() - started_at, 3),
            stdout_preview=_preview_text(process.stdout),
            stderr_preview=_preview_text(process.stderr),
        )
        return process

    def _capture_with_log(
            self,
            *,
            cmd: Sequence[str],
            log_path: Path,
            cwd: Optional[Path] = None,
            env: Optional[Dict[str, str]] = None,
    ) -> subprocess.CompletedProcess[str]:
        process = self._capture(cmd=cmd, cwd=cwd, env=env)
        cwd_path = cwd or self.root
        cmd_list = [str(item) for item in cmd]

        with log_path.open("a", encoding="utf-8") as fp:
            fp.write(f"$ {self._quote_cmd(cmd)}\n")
            fp.write(f"# cwd: {cwd_path}\n")
            if process.stdout:
                fp.write(process.stdout)
                if not process.stdout.endswith("\n"):
                    fp.write("\n")
            if process.stderr:
                fp.write(process.stderr)
                if not process.stderr.endswith("\n"):
                    fp.write("\n")
            fp.write(f"[exit] {process.returncode}\n\n")
        self._emit_state(
            "capture_logged",
            cmd=cmd_list,
            cmd_text=self._quote_cmd(cmd),
            returncode=process.returncode,
            log_path=_path_state(log_path),
        )
        return process

    def _clean_path(self, *, task: str, path: Path, log_path: Path) -> None:
        if not path.exists() and not path.is_symlink():
            return
        self._log(task, f"Cleaning path: {path}")
        self._emit_state(
            "clean_path",
            task=task,
            target=_path_state(path),
            log_path=_path_state(log_path),
        )
        with log_path.open("a", encoding="utf-8") as fp:
            fp.write(f"# clean path: {path}\n")
        if path.is_symlink() or path.is_file():
            path.unlink(missing_ok=True)
            return
        shutil.rmtree(path)

    def _reset_task_log(self, log_path: Path) -> None:
        if log_path.exists():
            log_path.unlink()
        log_path.parent.mkdir(parents=True, exist_ok=True)
        self._emit_state("reset_task_log", log_path=_path_state(log_path))

    def _stop_host_app_best_effort(self, *, task: str, log_path: Path, app_bin: Path) -> None:
        if self.system_name == "Windows":
            self._run(
                task=task,
                cmd=["taskkill", "/IM", "WhatSon.exe", "/F"],
                log_path=log_path,
                check=False,
            )
            return
        if shutil.which("pkill"):
            self._run(
                task=task,
                cmd=["pkill", "-f", str(app_bin)],
                log_path=log_path,
                check=False,
            )

    def _stop_host_processes_best_effort(self, *, task: str, log_path: Path) -> None:
        if self.system_name == "Windows":
            self._run(
                task=task,
                cmd=["taskkill", "/IM", "WhatSon.exe", "/F"],
                log_path=log_path,
                check=False,
            )
            return
        if shutil.which("pkill"):
            self._run(task=task, cmd=["pkill", "-x", "WhatSon"], log_path=log_path, check=False)
            self._run(
                task=task,
                cmd=["pkill", "-f", "WhatSon.app/Contents/MacOS/WhatSon"],
                log_path=log_path,
                check=False,
            )

    def _android_package_exists(self, *, adb_path: Path, serial: str, package_name: str) -> bool:
        package = package_name.strip()
        if not package:
            return False
        probe = self._capture(cmd=[str(adb_path), "-s", serial, "shell", "pm", "list", "packages", package])
        if probe.returncode != 0:
            return False
        token = f"package:{package}"
        return token in probe.stdout

    def _reset_android_package_best_effort(
            self,
            *,
            task: str,
            log_path: Path,
            adb_path: Path,
            package_name: str,
            serial: Optional[str] = None,
    ) -> None:
        package = package_name.strip()
        if not package:
            return
        adb_cmd = [str(adb_path)]
        if serial:
            adb_cmd.extend(["-s", serial])
        self._log(task, f"Resetting Android package state: {package}")
        self._run(
            task=task,
            cmd=adb_cmd + ["shell", "am", "force-stop", package],
            log_path=log_path,
            check=False,
        )
        self._run(
            task=task,
            cmd=adb_cmd + ["uninstall", package],
            log_path=log_path,
            check=False,
        )
        if serial:
            self._run(
                task=task,
                cmd=adb_cmd + ["shell", "pm", "clear", package],
                log_path=log_path,
                check=False,
            )
            self._run(
                task=task,
                cmd=adb_cmd + ["shell", "cmd", "package", "uninstall", "--user", "0", package],
                log_path=log_path,
                check=False,
            )
            self._run(
                task=task,
                cmd=adb_cmd + ["uninstall", package],
                log_path=log_path,
                check=False,
            )
            if self._android_package_exists(adb_path=adb_path, serial=serial, package_name=package):
                self._run(
                    task=task,
                    cmd=adb_cmd + ["shell", "pm", "uninstall", "--user", "0", package],
                    log_path=log_path,
                    check=False,
                )

    def _install_android_apk_with_retry(
            self,
            *,
            task: str,
            log_path: Path,
            adb_path: Path,
            apk_path: Path,
            package_name: str,
            attempts: int = 3,
    ) -> str:
        if attempts < 1:
            attempts = 1

        last_serial: Optional[str] = None
        for attempt in range(1, attempts + 1):
            serial = self._ensure_android_device(task=task, log_path=log_path)
            last_serial = serial
            adb_cmd = [str(adb_path), "-s", serial]
            install_cmd = adb_cmd + ["install", "-r", str(apk_path)]
            self._emit_state(
                "android_install_attempt",
                task=task,
                attempt=attempt,
                attempts=attempts,
                serial=serial,
                apk_path=_path_state(apk_path),
                package_name=package_name,
            )
            return_code = self._run(task=task, cmd=install_cmd, log_path=log_path, check=False)
            if return_code == 0:
                self._emit_state("android_install_result", task=task, attempt=attempt, status="success", serial=serial)
                return serial

            # Best-effort cleanup for stale package state (e.g. DELETE_FAILED_INTERNAL_ERROR)
            self._run(
                task=task,
                cmd=adb_cmd + ["shell", "am", "force-stop", package_name],
                log_path=log_path,
                check=False,
            )
            self._run(
                task=task,
                cmd=adb_cmd + ["uninstall", package_name],
                log_path=log_path,
                check=False,
            )
            self._run(
                task=task,
                cmd=adb_cmd + ["shell", "cmd", "package", "uninstall", "--user", "0", package_name],
                log_path=log_path,
                check=False,
            )
            if attempt < attempts:
                time.sleep(2)

        self._emit_state(
            "android_install_result",
            task=task,
            attempts=attempts,
            status="failed",
            last_serial=last_serial,
            apk_path=_path_state(apk_path),
            package_name=package_name,
        )
        raise CommandError(
            f"adb install failed after {attempts} attempts for '{apk_path}'"
            + (f" (last-serial={last_serial})" if last_serial else "")
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

    def _host_app_bundle(self, app_bin: Path) -> Optional[Path]:
        if self.system_name != "Darwin":
            return None
        current = app_bin
        for _ in range(4):
            if current.suffix == ".app" and current.exists():
                return current
            if current.parent == current:
                break
            current = current.parent
        return None

    def _read_bundle_id_from_app_bundle(self, app_bundle: Path) -> Optional[str]:
        info_plist = app_bundle / "Info.plist"
        if not info_plist.exists():
            return None
        try:
            with info_plist.open("rb") as fp:
                payload = plistlib.load(fp)
        except Exception:  # noqa: BLE001
            return None
        if not isinstance(payload, dict):
            return None
        bundle_id = payload.get("CFBundleIdentifier")
        if isinstance(bundle_id, str):
            cleaned = bundle_id.strip()
            if cleaned:
                return cleaned
        return None

    def _daemon_binary(self) -> Optional[Path]:
        candidates = [
            self.host_build_dir / "src" / "daemon" / "WhatSon_daemon",
            self.host_build_dir / "src" / "daemon" / "WhatSon_daemon.exe",
            self.host_build_dir / "src" / "daemon" / "whatson_daemon",
            self.host_build_dir / "src" / "daemon" / "whatson_daemon.exe",
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

    def _connected_android_devices(self, adb_path: Path) -> List[Tuple[str, bool]]:
        result = self._capture(cmd=[str(adb_path), "devices", "-l"])
        if result.returncode != 0:
            return []
        devices: List[Tuple[str, bool]] = []
        for line in result.stdout.splitlines():
            parts = line.strip().split()
            if len(parts) < 2 or parts[1] != "device":
                continue
            serial = parts[0]
            lowered = line.lower()
            is_emulator = (
                    serial.startswith("emulator-")
                    or "model:sdk_" in lowered
                    or "product:sdk_" in lowered
                    or " emulator" in lowered
            )
            devices.append((serial, is_emulator))
        return devices

    def _adb_devices_snapshot(self, adb_path: Path) -> str:
        probe = self._capture(cmd=[str(adb_path), "devices", "-l"])
        if probe.returncode != 0:
            return f"[adb devices failed: exit={probe.returncode}]"

        lines = [line.rstrip() for line in probe.stdout.splitlines()]
        if not lines:
            return "[adb devices output is empty]"
        return "\n".join(lines)

    def _connected_android_serial(self, adb_path: Path, *, physical_only: bool = True) -> Optional[str]:
        devices = self._connected_android_devices(adb_path)
        if physical_only:
            for serial, is_emulator in devices:
                if not is_emulator:
                    return serial
            return None
        if not devices:
            return None
        return devices[0][0]

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

        # Refresh adb state before device inspection.
        self._run(task=task, cmd=[str(adb_path), "start-server"], log_path=log_path, check=False)

        connected_devices = self._connected_android_devices(adb_path)
        self._emit_state(
            "android_device_scan",
            task=task,
            adb_path=_path_state(adb_path),
            devices=[
                {"serial": serial, "is_emulator": is_emulator}
                for serial, is_emulator in connected_devices
            ],
            android_allow_emulator=self.android_allow_emulator,
        )

        serial = self._connected_android_serial(adb_path, physical_only=True)
        if serial:
            self._log(task, f"Using connected Android physical device: {serial}")
            self._emit_state("android_device_ready", task=task, serial=serial, device_kind="physical")
            return serial

        snapshot = self._adb_devices_snapshot(adb_path)
        with log_path.open("a", encoding="utf-8") as fp:
            fp.write("# adb-devices-snapshot\n")
            fp.write(snapshot)
            if not snapshot.endswith("\n"):
                fp.write("\n")
            fp.write("\n")

        unauthorized_or_offline: List[str] = []
        for line in snapshot.splitlines():
            trimmed = line.strip()
            if not trimmed or trimmed.lower().startswith("list of devices attached"):
                continue
            tokens = trimmed.split()
            if len(tokens) < 2:
                continue
            serial_token = tokens[0]
            state = tokens[1].lower()
            lowered = trimmed.lower()
            is_emulator = (
                    serial_token.startswith("emulator-")
                    or "model:sdk_" in lowered
                    or "product:sdk_" in lowered
                    or " emulator" in lowered
            )
            if is_emulator:
                continue
            if state in {"unauthorized", "offline"}:
                unauthorized_or_offline.append(f"{serial_token}({state})")

        if unauthorized_or_offline:
            joined = ", ".join(unauthorized_or_offline)
            raise MissingPhysicalDeviceError(
                "Android physical device is connected but not ready: "
                f"{joined}. Unlock device and accept USB debugging authorization, then run build_all again."
            )

        if self.android_allow_emulator:
            for emulator_serial, is_emulator in connected_devices:
                if is_emulator:
                    self._log(task, f"Using running Android emulator: {emulator_serial}")
                    self._emit_state("android_device_ready", task=task, serial=emulator_serial, device_kind="emulator")
                    return emulator_serial

        if connected_devices and not self.android_allow_emulator:
            emulator_serials = [serial for serial, is_emulator in connected_devices if is_emulator]
            if emulator_serials:
                raise MissingPhysicalDeviceError(
                    "Only Android emulator was detected. "
                    "Connect a physical Android device (USB/Wi-Fi) and run build_all again."
                )

        if not self.android_allow_emulator:
            raise MissingPhysicalDeviceError(
                "No connected Android physical device was detected. "
                "Connect a physical Android device (USB/Wi-Fi) and run build_all again."
            )

        emulator_path = self._find_emulator_binary()
        if emulator_path is None:
            raise CommandError("Android emulator binary was not found.")

        avd = self._pick_avd(emulator_path)
        if not avd:
            raise CommandError("No Android AVD was found.")

        self._log(task, f"Starting Android emulator AVD: {avd}")
        self._emit_state(
            "android_emulator_launch",
            task=task,
            avd=avd,
            emulator_path=_path_state(emulator_path),
            log_path=_path_state(log_path.with_name(f"{log_path.stem}.emulator.log")),
        )
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
                serial = self._connected_android_serial(adb_path, physical_only=False)
                if serial:
                    self._log(task, f"Android emulator is ready: {serial}")
                    self._emit_state("android_device_ready", task=task, serial=serial, device_kind="emulator")
                    return serial
            time.sleep(2)

        raise CommandError("Android emulator boot timed out.")

    def _prepare_java21_env(self, env: Dict[str, str]) -> Dict[str, str]:
        if self.java21_home and self.java21_home.exists():
            updated = dict(env)
            updated["JAVA_HOME"] = str(self.java21_home)
            updated["JAVA21_HOME"] = str(self.java21_home)
            java_bin = self.java21_home / "bin"
            updated["PATH"] = f"{java_bin}{os.pathsep}{updated.get('PATH', '')}"
            return updated
        return env

    def _ios_sdk_path(self, sdk_name: str) -> Optional[Path]:
        if self.system_name != "Darwin":
            return None
        xcrun = shutil.which("xcrun")
        if not xcrun:
            return None
        probe = self._capture(cmd=[xcrun, "--sdk", sdk_name, "--show-sdk-path"])
        if probe.returncode != 0:
            return None
        sdk_path_raw = probe.stdout.strip()
        if not sdk_path_raw:
            return None
        sdk_path = Path(sdk_path_raw)
        if not sdk_path.exists():
            return None
        return sdk_path

    def _ios_backtrace_cmake_args(self, sdk_name: str) -> List[str]:
        # Some Qt iOS kits require explicit Backtrace cache hints during
        # cross-configure, otherwise Qt6Core marks WrapBacktrace unresolved.
        sdk_path = self._ios_sdk_path(sdk_name)
        if sdk_path is None:
            return []

        args: List[str] = []
        include_dir = sdk_path / "usr" / "include"
        libc_tbd = sdk_path / "usr" / "lib" / "libc.tbd"

        if include_dir.exists():
            args.append(f"-DBacktrace_INCLUDE_DIR={include_dir}")
        if libc_tbd.exists():
            args.append(f"-DBacktrace_LIBRARY={libc_tbd}")
            args.append(f"-DBacktrace_LIBRARIES={libc_tbd}")
        return args

    def _connected_ios_devices(self) -> List[Dict[str, str]]:
        if self.system_name != "Darwin":
            return []
        xcrun = shutil.which("xcrun")
        if not xcrun:
            return []

        probe = self._capture(cmd=[xcrun, "xcdevice", "list"])
        if probe.returncode != 0:
            return []

        try:
            payload = json.loads(probe.stdout)
        except Exception:  # noqa: BLE001
            return []
        if not isinstance(payload, list):
            return []

        devices: List[Dict[str, str]] = []
        for entry in payload:
            if not isinstance(entry, dict):
                continue
            if bool(entry.get("simulator", False)):
                continue
            if not bool(entry.get("available", False)):
                continue
            platform_name = str(entry.get("platform", "")).strip().lower()
            if "iphoneos" not in platform_name and "ipados" not in platform_name:
                continue
            identifier = str(entry.get("identifier", "")).strip()
            if not identifier:
                continue
            name = str(entry.get("name", "")).strip() or identifier
            architecture = str(entry.get("architecture", "")).strip().lower() or "arm64"
            if "arm64" in architecture:
                architecture = "arm64"
            devices.append(
                {
                    "identifier": identifier,
                    "name": name,
                    "platform": platform_name,
                    "architecture": architecture,
                }
            )
        return devices

    def _ensure_ios_device(self, *, task: str, log_path: Path) -> Dict[str, str]:
        devices = self._connected_ios_devices()
        self._emit_state(
            "ios_device_scan",
            task=task,
            requested_device=(self.ios_device or "").strip() or None,
            devices=devices,
            log_path=_path_state(log_path),
        )
        if not devices:
            raise CommandError(
                "No connected iOS physical device was detected. "
                "Connect an iPhone/iPad and trust this Mac, then run build_all again."
            )

        requested = (self.ios_device or "").strip()
        if requested:
            for device in devices:
                if requested in {device["identifier"], device["name"]}:
                    self._log(task, f"Using requested iOS device: {device['name']} ({device['identifier']})")
                    self._emit_state("ios_device_ready", task=task, device=device)
                    return device
            available = ", ".join(f"{item['name']}({item['identifier']})" for item in devices)
            raise CommandError(
                f"Requested iOS device '{requested}' was not found among connected devices: {available}"
            )

        selected = devices[0]
        self._log(task, f"Using connected iOS device: {selected['name']} ({selected['identifier']})")
        self._emit_state("ios_device_ready", task=task, device=selected)
        with log_path.open("a", encoding="utf-8") as fp:
            fp.write(f"# ios-device: {selected['name']} ({selected['identifier']})\n")
        return selected

    def _uninstall_ios_app_best_effort(
            self,
            *,
            task: str,
            log_path: Path,
            device_identifier: str,
            bundle_id: str,
    ) -> None:
        if self.system_name != "Darwin":
            return
        xcrun = shutil.which("xcrun")
        if not xcrun:
            return

        self._run(
            task=task,
            cmd=[
                xcrun,
                "devicectl",
                "device",
                "uninstall",
                "app",
                "--device",
                device_identifier,
                bundle_id,
            ],
            log_path=log_path,
            check=False,
        )

    def _clean_ios_simulator_app_best_effort(self, *, task: str, log_path: Path, bundle_id: str) -> None:
        if self.system_name != "Darwin":
            return
        xcrun = shutil.which("xcrun")
        if not xcrun:
            return
        devices = self._capture(cmd=[xcrun, "simctl", "list", "devices", "booted", "-j"])
        if devices.returncode != 0:
            return
        try:
            payload = json.loads(devices.stdout)
        except Exception:  # noqa: BLE001
            return
        for runtime_devices in payload.get("devices", {}).values():
            if not isinstance(runtime_devices, list):
                continue
            for device in runtime_devices:
                if not isinstance(device, dict):
                    continue
                udid = str(device.get("udid", "")).strip()
                if not udid:
                    continue
                self._run(
                    task=task,
                    cmd=[xcrun, "simctl", "terminate", udid, bundle_id],
                    log_path=log_path,
                    check=False,
                )
                self._run(
                    task=task,
                    cmd=[xcrun, "simctl", "uninstall", udid, bundle_id],
                    log_path=log_path,
                    check=False,
                )

    def _launch_ios_app_with_retry(
            self,
            *,
            task: str,
            log_path: Path,
            xcrun: str,
            ios_device_id: str,
            bundle_id: str,
            max_attempts: int = 24,
            interval_seconds: int = 5,
    ) -> None:
        launch_cmd = [
            xcrun,
            "devicectl",
            "device",
            "process",
            "launch",
            "--device",
            ios_device_id,
            "--terminate-existing",
            bundle_id,
        ]

        locked_markers = (
            "could not be unlocked",
            "device was not, or could not be, unlocked",
            "unable to launch",
            "locked",
            "requestdenied",
        )

        for attempt in range(1, max_attempts + 1):
            self._emit_state(
                "ios_launch_attempt",
                task=task,
                attempt=attempt,
                max_attempts=max_attempts,
                ios_device_id=ios_device_id,
                bundle_id=bundle_id,
            )
            result = self._capture_with_log(cmd=launch_cmd, log_path=log_path)
            if result.returncode == 0:
                if attempt > 1:
                    self._log(task, f"iOS launch succeeded after retry (attempt={attempt}).")
                self._emit_state("ios_launch_result", task=task, status="success", attempt=attempt, bundle_id=bundle_id)
                return

            output = f"{result.stdout}\n{result.stderr}".lower()
            if any(marker in output for marker in locked_markers):
                if attempt == 1:
                    self._log(
                        task,
                        "iOS device appears locked. Waiting for unlock and retrying app launch automatically.",
                    )
                    self._emit_state("ios_launch_waiting_for_unlock", task=task, attempt=attempt, bundle_id=bundle_id)
                if attempt < max_attempts:
                    time.sleep(interval_seconds)
                    continue
                raise CommandError(
                    "iOS app launch failed because the device stayed locked. "
                    "Unlock the device screen and run build_all again."
                )

            raise CommandError(
                f"iOS app launch failed (exit={result.returncode}) for bundle '{bundle_id}'. "
                f"See log: {log_path}"
            )

        raise CommandError(
            "iOS app launch retry limit was exceeded."
        )

    def _run_ios_xcodebuild_with_destination_fallback(
            self,
            *,
            task: str,
            log_path: Path,
            xcode_project: Path,
            derived_data_dir: Path,
            ios_device_id: str,
    ) -> None:
        primary_cmd = [
            "xcodebuild",
            *self._xcodebuild_job_args(task),
            "-project",
            str(xcode_project),
            "-scheme",
            "WhatSon",
            "-configuration",
            "Release",
            "-destination",
            f"id={ios_device_id}",
            "-derivedDataPath",
            str(derived_data_dir),
            "build",
        ]
        primary_result = self._capture_with_log(cmd=primary_cmd, log_path=log_path)
        if primary_result.returncode == 0:
            return

        output = f"{primary_result.stdout}\n{primary_result.stderr}".lower()
        missing_destination_markers = (
            "unable to find a destination matching the provided destination specifier",
            "available destinations for the \"whatson\" scheme",
        )
        if primary_result.returncode == 70 and all(marker in output for marker in missing_destination_markers):
            self._log(
                task,
                "xcodebuild destination id was not available. Retrying with generic iOS destination.",
            )
            self._emit_state(
                "ios_xcodebuild_destination_fallback",
                task=task,
                ios_device_id=ios_device_id,
                derived_data_dir=_path_state(derived_data_dir),
            )
            fallback_cmd = [
                "xcodebuild",
                *self._xcodebuild_job_args(task),
                "-project",
                str(xcode_project),
                "-scheme",
                "WhatSon",
                "-configuration",
                "Release",
                "-destination",
                "generic/platform=iOS",
                "-derivedDataPath",
                str(derived_data_dir),
                "build",
            ]
            fallback_result = self._capture_with_log(cmd=fallback_cmd, log_path=log_path)
            if fallback_result.returncode == 0:
                return
            raise CommandError(
                f"xcodebuild failed (exit={fallback_result.returncode}) after generic iOS fallback. "
                f"See log: {log_path}"
            )

        raise CommandError(
            f"xcodebuild failed (exit={primary_result.returncode}) for destination id={ios_device_id}. "
            f"See log: {log_path}"
        )

    def _host_platform_name(self) -> str:
        if self.system_name == "Darwin":
            return "macos"
        if self.system_name == "Windows":
            return "windows"
        return "linux"

    def _lvrs_platform_prefix(self, platform_name: str) -> Path:
        platform_prefix = self.lvrs_prefix / "platforms" / platform_name
        if platform_prefix.exists():
            return platform_prefix
        return self.lvrs_prefix

    def _lvrs_cmake_dir(self, prefix: Path) -> Path:
        return _resolve_package_cmake_dir(prefix, "LVRS")

    def _ensure_android_lvrs_prefix(self, *, task: str, log_path: Path) -> Path:
        root_android_prefix = self._lvrs_platform_prefix("android")
        root_android_config = self._lvrs_cmake_dir(root_android_prefix) / "LVRSConfig.cmake"
        if root_android_config.exists():
            return root_android_prefix

        cached_config = _resolve_package_cmake_dir(self.android_lvrs_prefix, "LVRS") / "LVRSConfig.cmake"
        if cached_config.exists():
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

        toolchain = _resolve_qt_toolchain_file(self.qt_android_prefix)
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
        self._run(
            task=task,
            cmd=["cmake", "--build", str(lvrs_build_dir), *self._build_parallel_args(task)],
            env=env,
            log_path=log_path,
        )
        self._run(task=task, cmd=["cmake", "--install", str(lvrs_build_dir)], env=env, log_path=log_path)

        installed_config = _resolve_package_cmake_dir(self.android_lvrs_prefix, "LVRS") / "LVRSConfig.cmake"
        if not installed_config.exists():
            raise CommandError(f"Android LVRS install failed: {installed_config} was not generated.")
        return self.android_lvrs_prefix

    def _ensure_ios_lvrs_prefix(self, *, task: str, log_path: Path) -> Path:
        ios_prefix = self.lvrs_prefix / "platforms" / "ios"
        ios_config = self._lvrs_cmake_dir(ios_prefix) / "LVRSConfig.cmake"
        ios_sdk_marker = ios_prefix / ".whatson_ios_sdk"
        if ios_config.exists() and ios_sdk_marker.exists():
            marker_value = ios_sdk_marker.read_text(encoding="utf-8").strip().lower()
            if marker_value == "iphoneos":
                return ios_prefix

        if not self.lvrs_source_dir.exists():
            raise CommandError(
                f"iOS LVRS prefix is missing and LVRS source dir was not found: {self.lvrs_source_dir}"
            )

        toolchain = _resolve_qt_toolchain_file(self.qt_ios_prefix)
        if not toolchain.exists():
            raise CommandError(f"Qt iOS toolchain file is missing: {toolchain}")

        lvrs_build_dir = self.lvrs_source_dir / "build-ios-install-device"
        self._clean_path(task=task, path=lvrs_build_dir, log_path=log_path)

        prefix_paths = [str(self.qt_ios_prefix)]
        if self.qt_host_prefix.exists():
            prefix_paths.append(str(self.qt_host_prefix))

        self._log(task, f"Building iOS LVRS package at: {ios_prefix}")
        self._run(
            task=task,
            cmd=[
                "cmake",
                "-G",
                "Xcode",
                "-S",
                str(self.lvrs_source_dir),
                "-B",
                str(lvrs_build_dir),
                "-DCMAKE_SYSTEM_NAME=iOS",
                f"-DCMAKE_TOOLCHAIN_FILE={toolchain}",
                f"-DCMAKE_PREFIX_PATH={';'.join(prefix_paths)}",
                "-DCMAKE_BUILD_TYPE=Release",
                "-DCMAKE_OSX_SYSROOT=iphoneos",
                "-DCMAKE_OSX_ARCHITECTURES=arm64",
                *self._ios_backtrace_cmake_args("iphoneos"),
                "-DCMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_ALLOWED=NO",
                "-DCMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED=NO",
                "-DLVRS_BUILD_EXAMPLES=OFF",
                "-DLVRS_BUILD_TESTS=OFF",
                "-DLVRS_BUILD_SHARED_LIBS=OFF",
                f"-DCMAKE_INSTALL_PREFIX={ios_prefix}",
            ],
            log_path=log_path,
        )
        self._run(
            task=task,
            cmd=["cmake", "--build", str(lvrs_build_dir), "--config", "Release", *self._build_parallel_args(task)],
            log_path=log_path,
        )
        self._run(
            task=task,
            cmd=["cmake", "--install", str(lvrs_build_dir), "--config", "Release"],
            log_path=log_path,
        )

        ios_config = self._lvrs_cmake_dir(ios_prefix) / "LVRSConfig.cmake"
        if not ios_config.exists():
            raise CommandError(f"iOS LVRS install failed: {ios_config} was not generated.")
        ios_sdk_marker.parent.mkdir(parents=True, exist_ok=True)
        ios_sdk_marker.write_text("iphoneos\n", encoding="utf-8")
        return ios_prefix

    def task_host(self) -> TaskResult:
        task = TASK_HOST
        log_path = self.logs_dir / f"{task}.log"
        self._emit_state(
            "task_prepare",
            task=task,
            log_path=_path_state(log_path),
            build_dir=_path_state(self.host_build_dir),
            root=_path_state(self.root),
            logs_dir=_path_state(self.logs_dir),
            no_host_run=self.no_host_run,
            qt_host_prefix=_path_state(self.qt_host_prefix),
            lvrs_prefix=_path_state(self.lvrs_prefix),
        )
        try:
            self._reset_task_log(log_path)
            self._stop_host_processes_best_effort(task=task, log_path=log_path)
            self._clean_path(task=task, path=self.host_build_dir, log_path=log_path)
            self.host_build_dir.mkdir(parents=True, exist_ok=True)

            cmake_cmd = ["cmake", "-S", str(self.root), "-B", str(self.host_build_dir)]
            prefix_paths: List[str] = []
            if self.qt_host_prefix.exists():
                prefix_paths.append(str(self.qt_host_prefix))
            if self.lvrs_prefix.exists():
                prefix_paths.append(str(self.lvrs_prefix))
            if prefix_paths:
                cmake_cmd.append(f"-DCMAKE_PREFIX_PATH={';'.join(prefix_paths)}")
            cmake_cmd.append("-DWHATSON_ENABLE_IOS_XCODEPROJ_ON_BUILD=OFF")
            lvrs_dir = self.lvrs_dir or self._lvrs_cmake_dir(
                self._lvrs_platform_prefix(self._host_platform_name())
            )
            if lvrs_dir.exists():
                cmake_cmd.append(f"-DLVRS_DIR={lvrs_dir}")

            self._run(task=task, cmd=cmake_cmd, log_path=log_path)
            self._run(
                task=task,
                cmd=[
                    "cmake",
                    "--build",
                    str(self.host_build_dir),
                    "--target",
                    "whatson_build_all",
                    *self._build_parallel_args(task),
                ],
                log_path=log_path,
            )

            daemon = self._daemon_binary()
            if daemon:
                self._run(task=task, cmd=[str(daemon), "--healthcheck"], log_path=log_path)

            if self.no_host_run:
                self._emit_state("task_finish", task=task, status="success",
                                 detail="Build completed (host run skipped by flag).")
                return TaskResult(task, "success", "Build completed (host run skipped by flag).", log_path)

            app_bin = self._host_app_binary()
            if app_bin is None:
                return TaskResult(task, "failed", "Build succeeded but host app binary was not found.", log_path)

            self._stop_host_app_best_effort(task=task, log_path=log_path, app_bin=app_bin)
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

            if self.system_name == "Darwin":
                app_bundle = self._host_app_bundle(app_bin)
                if app_bundle is not None and shutil.which("open"):
                    self._run(
                        task=task,
                        cmd=["open", "-a", str(app_bundle)],
                        log_path=log_path,
                        check=False,
                    )

            detail = f"Host app launched (pid={process.pid})."
            self._emit_state("task_finish", task=task, status="success", detail=detail, app_bin=_path_state(app_bin))
            return TaskResult(task, "success", detail, log_path)
        except Exception as exc:  # noqa: BLE001
            self._emit_state("task_finish", task=task, status="failed", detail=str(exc), log_path=_path_state(log_path))
            return TaskResult(task, "failed", str(exc), log_path)

    def task_ios(self) -> TaskResult:
        task = TASK_IOS
        log_path = self.logs_dir / f"{task}.log"
        if self.system_name != "Darwin":
            self._emit_state("task_finish", task=task, status="skipped",
                             detail="iOS Xcode project generation is macOS-only.")
            return TaskResult(task, "skipped", "iOS Xcode project generation is macOS-only.", log_path)

        self._emit_state(
            "task_prepare",
            task=task,
            log_path=_path_state(log_path),
            project_dir=_path_state(self.ios_project_dir),
            root=_path_state(self.root),
            ios_bundle_id=self.ios_bundle_id,
            ios_device=self.ios_device,
            qt_ios_prefix=_path_state(self.qt_ios_prefix),
            lvrs_prefix=_path_state(self.lvrs_prefix),
        )
        try:
            self._reset_task_log(log_path)
            ios_device = self._ensure_ios_device(task=task, log_path=log_path)
            ios_device_id = ios_device["identifier"]
            ios_arch = ios_device.get("architecture", "arm64") or "arm64"
            self._uninstall_ios_app_best_effort(
                task=task,
                log_path=log_path,
                device_identifier=ios_device_id,
                bundle_id=self.ios_bundle_id,
            )
            self._clean_path(task=task, path=self.ios_project_dir, log_path=log_path)
            toolchain = _resolve_qt_toolchain_file(self.qt_ios_prefix)
            if not toolchain.exists():
                raise CommandError(f"Qt iOS toolchain file was not found: {toolchain}")

            ios_lvrs_prefix = self._ensure_ios_lvrs_prefix(task=task, log_path=log_path)
            prefix_paths = [str(self.qt_ios_prefix), str(ios_lvrs_prefix)]
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
                "-DCMAKE_OSX_SYSROOT=iphoneos",
                f"-DCMAKE_OSX_ARCHITECTURES={ios_arch}",
                *self._ios_backtrace_cmake_args("iphoneos"),
                "-DCMAKE_XCODE_ATTRIBUTE_CODE_SIGN_STYLE=Automatic",
                f"-DCMAKE_XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER={self.ios_bundle_id}",
                "-DCMAKE_DISABLE_FIND_PACKAGE_Qt6GrpcQuick=TRUE",
                "-DCMAKE_DISABLE_FIND_PACKAGE_Qt6ProtobufQuick=TRUE",
            ]
            if self.ios_development_team:
                ios_cmd.append(f"-DCMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM={self.ios_development_team}")
            if self.ios_code_sign_identity:
                ios_cmd.append(f"-DCMAKE_XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY={self.ios_code_sign_identity}")

            lvrs_dir = self._lvrs_cmake_dir(ios_lvrs_prefix)
            if lvrs_dir.exists():
                ios_cmd.append(f"-DLVRS_DIR={lvrs_dir}")

            self._run(task=task, cmd=ios_cmd, log_path=log_path)

            xcode_project = self.ios_project_dir / "WhatSon.xcodeproj"
            if not xcode_project.exists():
                raise CommandError(f"Xcode project was not generated: {xcode_project}")

            derived_data_dir = self.ios_project_dir / "derived-data"
            self._clean_path(task=task, path=derived_data_dir, log_path=log_path)

            self._run_ios_xcodebuild_with_destination_fallback(
                task=task,
                log_path=log_path,
                xcode_project=xcode_project,
                derived_data_dir=derived_data_dir,
                ios_device_id=ios_device_id,
            )

            app_bundle_candidates = [
                derived_data_dir / "Build" / "Products" / "Release-iphoneos" / "WhatSon.app",
                self.ios_project_dir / "src" / "app" / "bin" / "Release" / "WhatSon.app",
                self.ios_project_dir / "src" / "app" / "bin" / "Debug" / "WhatSon.app",
            ]
            app_bundle: Path = app_bundle_candidates[0]
            for candidate in app_bundle_candidates:
                if candidate.exists():
                    app_bundle = candidate
                    break
            if not app_bundle.exists():
                candidates = sorted(derived_data_dir.glob("Build/Products/*-iphoneos/WhatSon.app"))
                if candidates:
                    app_bundle = candidates[-1]
            if not app_bundle.exists():
                candidates = sorted((self.ios_project_dir / "src" / "app" / "bin").glob("**/WhatSon.app"))
                if candidates:
                    app_bundle = candidates[-1]
            if not app_bundle.exists():
                raise CommandError("iOS .app bundle was not generated for iphoneos.")

            detected_bundle_id = self._read_bundle_id_from_app_bundle(app_bundle)
            launch_bundle_id = detected_bundle_id or self.ios_bundle_id
            if detected_bundle_id and detected_bundle_id != self.ios_bundle_id:
                self._log(
                    task,
                    "Configured iOS bundle id differs from built app bundle id. "
                    f"Using built bundle id for launch: {detected_bundle_id}",
                )
                with log_path.open("a", encoding="utf-8") as fp:
                    fp.write(
                        "# bundle-id-mismatch: "
                        f"configured={self.ios_bundle_id}, built={detected_bundle_id}\n"
                    )

            xcrun = shutil.which("xcrun")
            if not xcrun:
                raise CommandError("xcrun was not found, cannot install .app on iOS device.")

            self._run(
                task=task,
                cmd=[
                    xcrun,
                    "devicectl",
                    "device",
                    "install",
                    "app",
                    "--device",
                    ios_device_id,
                    str(app_bundle),
                ],
                log_path=log_path,
            )
            self._launch_ios_app_with_retry(
                task=task,
                log_path=log_path,
                xcrun=xcrun,
                ios_device_id=ios_device_id,
                bundle_id=launch_bundle_id,
            )

            detail = f"Installed .app on iOS device {ios_device['name']} ({ios_device_id}): {app_bundle}"
            self._emit_state("task_finish", task=task, status="success", detail=detail,
                             app_bundle=_path_state(app_bundle))
            return TaskResult(task, "success", detail, log_path)
        except Exception as exc:  # noqa: BLE001
            self._emit_state("task_finish", task=task, status="failed", detail=str(exc), log_path=_path_state(log_path))
            return TaskResult(task, "failed", str(exc), log_path)

    def task_android(self) -> TaskResult:
        task = TASK_ANDROID
        log_path = self.logs_dir / f"{task}.log"
        self._emit_state(
            "task_prepare",
            task=task,
            log_path=_path_state(log_path),
            build_dir=_path_state(self.android_build_dir),
            studio_dir=_path_state(self.android_studio_dir),
            root=_path_state(self.root),
            android_package=self.android_package,
            android_allow_emulator=self.android_allow_emulator,
            android_sdk_root=_path_state(self.android_sdk_root),
            android_ndk_root=_path_state(self.android_ndk_root),
            qt_android_prefix=_path_state(self.qt_android_prefix),
            lvrs_prefix=_path_state(self.lvrs_prefix),
        )
        try:
            self._reset_task_log(log_path)
            adb_path = self._find_adb()
            if adb_path is None:
                raise CommandError("adb was not found.")

            serial = self._ensure_android_device(task=task, log_path=log_path)
            _ = serial

            self._clean_path(task=task, path=self.android_build_dir, log_path=log_path)
            self._clean_path(task=task, path=self.android_studio_dir, log_path=log_path)
            cleanup_packages = {self.android_package}
            for package_name in sorted(cleanup_packages):
                self._reset_android_package_best_effort(
                    task=task,
                    log_path=log_path,
                    adb_path=adb_path,
                    package_name=package_name,
                    serial=serial,
                )

            if self.system_name == "Darwin":
                result = self._task_android_macos_proven(
                    task=task,
                    log_path=log_path,
                    adb_path=adb_path,
                    serial=serial,
                )
                if result:
                    self._emit_state("task_finish", task=task, status="success", detail=result)
                    return TaskResult(task, "success", result, log_path)
            else:
                result = self._task_android_generic(task=task, log_path=log_path)
                if result:
                    self._emit_state("task_finish", task=task, status="success", detail=result)
                    return TaskResult(task, "success", result, log_path)

            self._emit_state("task_finish", task=task, status="failed", detail="Android launch did not complete.")
            return TaskResult(task, "failed", "Android launch did not complete.", log_path)
        except MissingPhysicalDeviceError as exc:
            self._emit_state("task_finish", task=task, status="skipped", detail=str(exc),
                             log_path=_path_state(log_path))
            return TaskResult(task, "skipped", str(exc), log_path)
        except Exception as exc:  # noqa: BLE001
            self._emit_state("task_finish", task=task, status="failed", detail=str(exc), log_path=_path_state(log_path))
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
        lvrs_dir = self.lvrs_dir or self._lvrs_cmake_dir(
            self._lvrs_platform_prefix(self._host_platform_name())
        )
        if lvrs_dir.exists():
            cmake_cmd.append(f"-DLVRS_DIR={lvrs_dir}")

        self._run(task=task, cmd=cmake_cmd, log_path=log_path)
        self._run(
            task=task,
            cmd=[
                "cmake",
                "--build",
                str(self.android_build_dir),
                "--target",
                "launch_WhatSon_android",
                *self._build_parallel_args(task),
            ],
            log_path=log_path,
        )
        gradle_dir = self._find_android_gradle_dir()
        if gradle_dir is None:
            return "launch_WhatSon_android completed."
        studio_dir = self._export_android_studio_project(source_dir=gradle_dir, task=task, log_path=log_path)
        return f"launch_WhatSon_android completed. Android Studio project: {studio_dir}"

    def _task_android_macos_proven(
            self,
            *,
            task: str,
            log_path: Path,
            adb_path: Path,
            serial: str,
    ) -> Optional[str]:
        if not self.qt_android_prefix.exists():
            raise CommandError(f"Qt Android prefix was not found: {self.qt_android_prefix}")
        if not self.qt_host_prefix.exists():
            raise CommandError(f"Qt host prefix was not found: {self.qt_host_prefix}")
        if not self.android_sdk_root.exists():
            raise CommandError(f"Android SDK root was not found: {self.android_sdk_root}")
        if self.android_ndk_root is None or not self.android_ndk_root.exists():
            raise CommandError("Android NDK root was not found.")

        android_lvrs = self._ensure_android_lvrs_prefix(task=task, log_path=log_path)
        toolchain = _resolve_qt_toolchain_file(self.qt_android_prefix)
        if not toolchain.exists():
            raise CommandError(f"Qt Android toolchain file was not found: {toolchain}")

        self.android_build_dir.mkdir(parents=True, exist_ok=True)
        prefix_paths = [str(self.qt_android_prefix), str(android_lvrs), str(self.qt_host_prefix)]
        android_lvrs_dir = self._lvrs_cmake_dir(android_lvrs)

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
                f"-DLVRS_DIR={android_lvrs_dir}",
            ],
            env=env,
            log_path=log_path,
        )

        self._run(
            task=task,
            cmd=[
                "cmake",
                "--build",
                str(self.android_build_dir),
                "--target",
                "WhatSon_make_apk",
                *self._build_parallel_args(task),
            ],
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
            gradle_cmd = [str(gradlew), *self._gradle_job_args(task), "assembleDebug"]
        elif gradlew_bat.exists():
            gradle_cmd = [str(gradlew_bat), *self._gradle_job_args(task), "assembleDebug"]
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

        if launch_package != self.android_package:
            self._reset_android_package_best_effort(
                task=task,
                log_path=log_path,
                adb_path=adb_path,
                package_name=launch_package,
                serial=serial,
            )
        install_serial = self._install_android_apk_with_retry(
            task=task,
            log_path=log_path,
            adb_path=adb_path,
            apk_path=debug_apk,
            package_name=launch_package,
            attempts=3,
        )
        adb_target = [str(adb_path), "-s", install_serial]
        self._run(
            task=task,
            cmd=adb_target + ["shell", "am", "force-stop", launch_package],
            log_path=log_path,
            check=False,
        )
        self._run(
            task=task,
            cmd=adb_target + ["shell", "monkey", "-p", launch_package, "-c", "android.intent.category.LAUNCHER", "1"],
            log_path=log_path,
        )
        studio_dir = self._export_android_studio_project(source_dir=gradle_dir, task=task, log_path=log_path)

        pid_probe = self._capture(cmd=adb_target + ["shell", "pidof", launch_package])
        pid_text = pid_probe.stdout.strip() if pid_probe.returncode == 0 else ""
        if pid_text:
            return (
                f"Installed and launched on Android device ({launch_package}, pid={pid_text}). "
                f"Android Studio project: {studio_dir}"
            )
        return f"Installed and launched on Android device ({launch_package}). Android Studio project: {studio_dir}"

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

        self._emit_state(
            "runner_start",
            requested_tasks=list(tasks),
            selected_tasks=selected,
            mode="sequential" if self.sequential else "parallel",
            build_jobs=self.build_jobs,
            root=_path_state(self.root),
            logs_dir=_path_state(self.logs_dir),
        )

        if self.sequential:
            self._task_job_limit_map = {name: self.build_jobs for name in selected}
            try:
                self._emit_state("runner_job_budget", task_job_limits=self._task_job_limit_map)
                results = [task_map[name]() for name in selected]
                self._emit_state(
                    "runner_finish",
                    results=[
                        {"task": result.name, "status": result.status, "detail": result.detail}
                        for result in results
                    ],
                )
                return results
            finally:
                self._task_job_limit_map = {}

        worker_count = _parallel_worker_count(len(selected), self.build_jobs)
        self._task_job_limit_map = _task_job_limits(selected, self.build_jobs)
        self._emit_state(
            "runner_job_budget",
            worker_count=worker_count,
            task_job_limits=self._task_job_limit_map,
        )

        results: Dict[str, TaskResult] = {}
        try:
            with ThreadPoolExecutor(max_workers=worker_count) as executor:
                future_map = {executor.submit(task_map[name]): name for name in selected}
                for future in as_completed(future_map):
                    name = future_map[future]
                    try:
                        results[name] = future.result()
                    except Exception as exc:  # noqa: BLE001
                        results[name] = TaskResult(name, "failed", str(exc), self.logs_dir / f"{name}.log")
        finally:
            self._task_job_limit_map = {}

        ordered = [results[name] for name in selected]
        self._emit_state(
            "runner_finish",
            results=[
                {"task": result.name, "status": result.status, "detail": result.detail}
                for result in ordered
            ],
        )
        return ordered


def parse_args() -> argparse.Namespace:
    home = _expand("~")
    system_name = platform.system()
    repo_root = Path(__file__).resolve().parents[1]

    pre_parser = argparse.ArgumentParser(add_help=False)
    pre_parser.add_argument(
        "--dev-env-json",
        default=os.environ.get("WHATSON_DEV_ENV_JSON", str(repo_root / "build" / "dev-env" / "dev_env.json")),
        help="Path to dev_env.py output JSON profile.",
    )
    pre_args, _ = pre_parser.parse_known_args()
    dev_env_profile = _load_dev_env_profile(_expand(pre_args.dev_env_json))

    qt_root = _expand(_pick_setting("QT_VERSION_ROOT", dev_env_profile, str(_default_qt_version_root(home))))
    qt_host_default = _default_qt_host_prefix(system_name, qt_root)
    qt_android_default = _default_qt_android_prefix(qt_root)
    android_sdk = _expand(
        _pick_setting("ANDROID_SDK_ROOT", dev_env_profile, str(_default_android_sdk(system_name, home))))
    lvrs_host_default = _expand(
        _pick_setting("LVRS_PREFIX", dev_env_profile, str(_default_lvrs_prefix_for_system(system_name, home)))
    )
    lvrs_base_prefix = lvrs_host_default
    lvrs_android_default = _default_lvrs_android_prefix(lvrs_base_prefix, home)
    lvrs_source_default = _expand(
        _pick_setting("LVRS_SOURCE_DIR", dev_env_profile, str(_default_lvrs_source_dir(home, repo_root)))
    )
    java21_detected = _detect_java21_home(system_name)
    java21_default = str(java21_detected) if java21_detected else None

    parser = argparse.ArgumentParser(
        parents=[pre_parser],
        description=(
            "Build and run WhatSon on the current host, launch on Android (emulator fallback enabled), "
            "build/install .app on a connected iOS physical device, and export project artifacts."
        )
    )
    parser.add_argument("--root", default=str(repo_root), help="Repository root path.")
    parser.add_argument("--logs-dir", default=str(repo_root / "build" / "automation-logs"))
    parser.add_argument("--tasks", default="host,android,ios", help="Comma-separated tasks: host,android,ios")

    mode_group = parser.add_mutually_exclusive_group()
    mode_group.add_argument(
        "--sequential",
        action="store_true",
        help="Run tasks sequentially (default).",
    )
    mode_group.add_argument(
        "--parallel",
        action="store_true",
        help="Run tasks in parallel and split the build job budget across active tasks.",
    )
    parser.add_argument(
        "--jobs",
        type=_positive_int_arg,
        default=_default_build_jobs(),
        help="Maximum number of native build jobs to use across active tasks (default: %(default)s).",
    )
    parser.add_argument("--no-host-run", action="store_true", help="Skip launching the host desktop app.")

    parser.add_argument("--host-build-dir", default=str(repo_root / "build" / "host-auto"))
    parser.add_argument("--android-build-dir",
                        default=str(repo_root / "build" / "android-auto"))
    parser.add_argument("--ios-project-dir",
                        default=str(repo_root / "build" / "ios-xcode-artifact"))
    parser.add_argument(
        "--android-studio-dir",
        default=str(repo_root / "build" / "android-studio-artifact"),
        help="Exported Android Studio project path.",
    )

    parser.add_argument("--qt-version-root", default=str(qt_root))
    parser.add_argument("--qt-host-prefix",
                        default=str(_expand(_pick_setting("QT_HOST_PREFIX", dev_env_profile, str(qt_host_default)))))
    parser.add_argument(
        "--qt-ios-prefix",
        default=str(_expand(_pick_setting("QT_IOS_PREFIX", dev_env_profile, str(qt_root / "ios")))),
    )
    parser.add_argument(
        "--qt-android-prefix",
        default=str(_expand(_pick_setting("QT_ANDROID_PREFIX", dev_env_profile, str(qt_android_default)))),
    )

    parser.add_argument(
        "--lvrs-prefix",
        default=str(_expand(_pick_setting("LVRS_PREFIX", dev_env_profile, str(lvrs_host_default)))),
        help="LVRS root prefix (platform-specific package is auto-dispatched by LVRS).",
    )
    parser.add_argument(
        "--lvrs-dir",
        default=_pick_setting("LVRS_DIR", dev_env_profile),
        help="Path to LVRS CMake package directory (blueprint/cmake/LVRS or lib/cmake/LVRS).",
    )
    parser.add_argument(
        "--android-lvrs-prefix",
        default=str(_expand(_pick_setting("LVRS_ANDROID_PREFIX", dev_env_profile, str(lvrs_android_default)))),
        help="LVRS prefix for Android configure.",
    )
    parser.add_argument("--lvrs-source-dir", default=str(lvrs_source_default))
    parser.add_argument("--skip-android-lvrs-build", action="store_true")

    parser.add_argument("--android-sdk-root", default=str(android_sdk))
    parser.add_argument("--android-ndk-root", default=_pick_setting("ANDROID_NDK_ROOT", dev_env_profile))
    parser.add_argument("--android-avd", default=_pick_setting("ANDROID_AVD", dev_env_profile))
    parser.add_argument(
        "--android-allow-emulator",
        dest="android_allow_emulator",
        action="store_true",
        default=True,
        help="Allow Android emulator fallback when no physical device is connected (default: enabled).",
    )
    parser.add_argument(
        "--android-physical-only",
        dest="android_allow_emulator",
        action="store_false",
        help="Disable emulator fallback and require a connected Android physical device.",
    )
    parser.add_argument(
        "--android-package",
        default=_pick_setting("WHATSON_ANDROID_PACKAGE", dev_env_profile),
        help="Android application id/package. Auto-detected from platform AndroidManifest when omitted.",
    )
    parser.add_argument(
        "--ios-bundle-id",
        default=_pick_setting("WHATSON_APPLE_BUNDLE_ID", dev_env_profile, DEFAULT_APPLE_BUNDLE_ID),
        help="iOS bundle identifier used for .app install/launch on connected physical device.",
    )
    parser.add_argument(
        "--ios-device",
        default=_pick_setting("WHATSON_IOS_DEVICE", dev_env_profile),
        help="Target iOS physical device identifier or device name. Auto-selects first connected device when omitted.",
    )
    parser.add_argument(
        "--ios-development-team",
        default=_pick_setting("WHATSON_IOS_DEVELOPMENT_TEAM", dev_env_profile),
        help="Optional Apple Development Team ID for iOS code signing.",
    )
    parser.add_argument(
        "--ios-code-sign-identity",
        default=_pick_setting("WHATSON_IOS_CODE_SIGN_IDENTITY", dev_env_profile),
        help="Optional iOS code signing identity (for example: Apple Development).",
    )
    parser.add_argument(
        "--java21-home",
        default=_pick_setting("JAVA21_HOME", dev_env_profile, java21_default),
    )

    args = parser.parse_args()
    if not args.sequential and not args.parallel:
        args.sequential = True
    return args


def main() -> int:
    args = parse_args()
    tasks = [item.strip() for item in args.tasks.split(",") if item.strip()]
    runner = BuildAll(args)

    emit_state(
        "build_platform_runner",
        "script_entry",
        root=_path_state(runner.root),
        tasks=tasks,
        mode="sequential" if runner.sequential else "parallel",
        jobs=runner.build_jobs,
        logs_dir=_path_state(runner.logs_dir),
    )
    print(f"[build_all] root={runner.root}", flush=True)
    print(f"[build_all] tasks={','.join(tasks)}", flush=True)
    print(f"[build_all] mode={'sequential' if runner.sequential else 'parallel'}", flush=True)
    print(f"[build_all] jobs={runner.build_jobs}", flush=True)

    try:
        results = runner.run(tasks)
    except Exception as exc:  # noqa: BLE001
        emit_state("build_platform_runner", "script_finish", status="failed", detail=str(exc))
        print(f"[build_all] fatal: {exc}", flush=True)
        return 1

    has_failure = False
    for result in results:
        print(f"[{result.name}] {result.status}: {result.detail}", flush=True)
        print(f"[{result.name}] log: {result.log_path}", flush=True)
        if result.status == "failed":
            has_failure = True

    emit_state("build_platform_runner", "script_finish", status="failed" if has_failure else "success")
    return 1 if has_failure else 0


if __name__ == "__main__":
    sys.exit(main())
