#!/usr/bin/env python3
from __future__ import annotations

import argparse
import json
import os
import platform
import shlex
import shutil
import subprocess
import sys
import time
from dataclasses import dataclass
from pathlib import Path
from typing import Dict, Iterable, List, Optional, Sequence, Tuple

TASK_HOST = "host"
ALL_TASKS: Tuple[str, ...] = (TASK_HOST,)
DEFAULT_BUILD_JOBS_CAP = 8
STATE_TEXT_PREVIEW_LIMIT = 240
STATE_ENV_PREFIXES = ("CMAKE_", "LVRS", "QT", "WHATSON")


@dataclass
class TaskResult:
    name: str
    status: str
    detail: str
    log_path: Path


class CommandError(RuntimeError):
    pass


def _expand(path: str | Path) -> Path:
    return Path(os.path.expandvars(os.path.expanduser(str(path)))).resolve()


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


def emit_state(script: str, event: str, **fields) -> None:
    payload = {
        "ts": _timestamp_now(),
        "pid": os.getpid(),
        "cwd": str(Path.cwd()),
        "script": script,
        "event": event,
    }
    for key, value in fields.items():
        if value is not None:
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
    _ = total_jobs
    return 1 if task_count > 0 else 0


def _task_job_limits(tasks: Sequence[str], total_jobs: int) -> Dict[str, int]:
    return {task: total_jobs for task in tasks}


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


def _find_existing(paths: Iterable[Path]) -> Optional[Path]:
    for path in paths:
        if path.exists():
            return path
    return None


def _latest_version_dir(parent: Path, pattern: str = r"^\d+(\.\d+)*$") -> Optional[Path]:
    if not parent.is_dir():
        return None
    import re

    regex = re.compile(pattern)
    candidates = [item for item in parent.iterdir() if item.is_dir() and regex.match(item.name)]
    if not candidates:
        return None

    def version_key(path: Path) -> Tuple[int, ...]:
        return tuple(int(part) for part in re.findall(r"\d+", path.name))

    return sorted(candidates, key=version_key)[-1]


def _default_qt_version_root(home: Path) -> Path:
    qt_home = home / "Qt"
    return _latest_version_dir(qt_home) or qt_home


def _default_qt_host_prefix(system_name: str, qt_root: Path) -> Path:
    if system_name == "Darwin":
        candidates = [qt_root / "macos"]
    elif system_name == "Windows":
        candidates = [qt_root / "msvc2022_64", qt_root / "msvc2019_64", qt_root / "mingw_64"]
    else:
        candidates = [qt_root / "gcc_64"]
    return _find_existing(candidates) or candidates[0]


def _default_lvrs_prefix(system_name: str, home: Path) -> Path:
    _ = system_name
    return _find_existing([home / ".local" / "LVRS", Path("/local/LVRS")]) or home / ".local" / "LVRS"


def _default_lvrs_source_dir(home: Path, repo_root: Path) -> Path:
    candidates = [
        home / "Developer" / "LVRS",
        repo_root.parent / "LVRS",
        repo_root.parent.parent / "LVRS",
        repo_root.parent.parent / "InfraSystem" / "LVRS",
        Path("/local/LVRS"),
    ]
    for candidate in candidates:
        if (candidate / "CMakeLists.txt").exists():
            return candidate
    return home / "Developer" / "LVRS"


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
    return {str(key): str(value) for key, value in env_map.items() if isinstance(key, str) and value is not None}


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
        self.lvrs_prefix = _expand(args.lvrs_prefix)
        self.lvrs_dir = _expand(args.lvrs_dir) if args.lvrs_dir else None
        self.lvrs_source_dir = _expand(args.lvrs_source_dir)
        self.host_build_dir = _expand(args.host_build_dir)
        self.trial_build_dir = _expand(args.trial_build_dir)
        self.no_host_run = args.no_host_run
        self.sequential = args.sequential
        self.build_jobs = args.jobs

    def _emit_state(self, event: str, **fields) -> None:
        emit_state("build_platform_runner", event, **fields)

    def _log(self, task: str, message: str) -> None:
        print(f"[{task}] {message}", flush=True)

    def _quote_cmd(self, cmd: Sequence[str]) -> str:
        return " ".join(shlex.quote(str(item)) for item in cmd)

    def _host_platform_name(self) -> str:
        if self.system_name == "Darwin":
            return "macos"
        if self.system_name == "Windows":
            return "windows"
        return "linux"

    def _lvrs_platform_prefix(self) -> Path:
        platform_prefix = self.lvrs_prefix / "platforms" / self._host_platform_name()
        if platform_prefix.exists():
            return platform_prefix
        return self.lvrs_prefix

    def _lvrs_cmake_dir(self, prefix: Path) -> Path:
        candidates = [
            prefix / "blueprint" / "cmake" / "LVRS",
            prefix / "lib" / "cmake" / "LVRS",
            self.lvrs_prefix / "blueprint" / "cmake" / "LVRS",
            self.lvrs_prefix / "lib" / "cmake" / "LVRS",
        ]
        return _find_existing(candidates) or candidates[0]

    def _host_configure_command(self, build_dir: Path) -> List[str]:
        cmake_cmd = ["cmake", "-S", str(self.root), "-B", str(build_dir)]
        prefix_paths: List[str] = []
        if self.qt_host_prefix.exists():
            prefix_paths.append(str(self.qt_host_prefix))
        if self.lvrs_prefix.exists():
            prefix_paths.append(str(self.lvrs_prefix))
        if prefix_paths:
            cmake_cmd.append(f"-DCMAKE_PREFIX_PATH={';'.join(prefix_paths)}")
        lvrs_dir = self.lvrs_dir or self._lvrs_cmake_dir(self._lvrs_platform_prefix())
        if lvrs_dir.exists():
            cmake_cmd.append(f"-DLVRS_DIR={lvrs_dir}")
        return cmake_cmd

    def _run(
            self,
            *,
            task: str,
            cmd: Sequence[str],
            log_path: Path,
            cwd: Optional[Path] = None,
            env: Optional[Dict[str, str]] = None,
            check: bool = True,
    ) -> int:
        cwd_path = cwd or self.root
        merged_env = os.environ.copy()
        if env:
            merged_env.update(env)
        log_path.parent.mkdir(parents=True, exist_ok=True)
        cmd_list = [str(item) for item in cmd]
        cmd_text = self._quote_cmd(cmd)
        started_at = time.monotonic()
        self._emit_state(
            "command_start",
            task=task,
            cmd=cmd_list,
            cmd_text=cmd_text,
            cwd=_path_state(cwd_path),
            log_path=_path_state(log_path),
            env_overrides=_env_debug_overrides(merged_env) or None,
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
        self._emit_state(
            "command_finish",
            task=task,
            cmd=cmd_list,
            cmd_text=cmd_text,
            returncode=process.returncode,
            duration_seconds=round(time.monotonic() - started_at, 3),
            log_path=_path_state(log_path),
        )
        if check and process.returncode != 0:
            raise CommandError(f"Command failed ({process.returncode}): {cmd_text}")
        return process.returncode

    def _clean_path(self, *, task: str, path: Path, log_path: Path) -> None:
        if not path.exists() and not path.is_symlink():
            return
        self._log(task, f"Cleaning path: {path}")
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

    def _host_app_binary(self) -> Optional[Path]:
        candidates: List[Path] = []
        if self.system_name == "Darwin":
            candidates.append(self.host_build_dir / "WhatSon.app" / "Contents" / "MacOS" / "WhatSon")
            candidates.append(
                self.host_build_dir / "src" / "app" / "bin" / "WhatSon.app" / "Contents" / "MacOS" / "WhatSon")
        elif self.system_name == "Windows":
            candidates.extend([
                self.host_build_dir / "WhatSon.exe",
                self.host_build_dir / "src" / "app" / "bin" / "WhatSon.exe",
                self.host_build_dir / "src" / "app" / "WhatSon.exe",
            ])
        else:
            candidates.extend([
                self.host_build_dir / "WhatSon",
                self.host_build_dir / "src" / "app" / "bin" / "WhatSon",
                self.host_build_dir / "src" / "app" / "WhatSon",
            ])
        return _find_existing(candidates)

    def _build_trial_package(self, *, task: str, log_path: Path) -> None:
        self._clean_path(task=task, path=self.trial_build_dir, log_path=log_path)
        self.trial_build_dir.mkdir(parents=True, exist_ok=True)
        self._run(task=task, cmd=self._host_configure_command(self.trial_build_dir), log_path=log_path)
        self._run(
            task=task,
            cmd=[
                "cmake",
                "--build",
                str(self.trial_build_dir),
                "--target",
                "whatson_package",
                "--parallel",
                str(self.build_jobs),
            ],
            log_path=log_path,
        )

    def _host_desktop_app_launch_enabled(self) -> bool:
        if self.no_host_run:
            return False
        if self.system_name != "Linux":
            return True
        return bool(os.environ.get("DISPLAY") or os.environ.get("WAYLAND_DISPLAY"))

    def task_host(self) -> TaskResult:
        task = TASK_HOST
        log_path = self.logs_dir / "host.log"
        self._reset_task_log(log_path)
        self._emit_state(
            "task_start",
            task=task,
            build_dir=_path_state(self.host_build_dir),
            trial_build_dir=_path_state(self.trial_build_dir),
            qt_host_prefix=_path_state(self.qt_host_prefix),
            lvrs_prefix=_path_state(self.lvrs_prefix),
        )
        try:
            self._clean_path(task=task, path=self.host_build_dir, log_path=log_path)
            self.host_build_dir.mkdir(parents=True, exist_ok=True)
            self._run(task=task, cmd=self._host_configure_command(self.host_build_dir), log_path=log_path)
            self._run(
                task=task,
                cmd=[
                    "cmake",
                    "--build",
                    str(self.host_build_dir),
                    "--target",
                    "whatson_build_regression",
                    "--parallel",
                    str(self.build_jobs),
                ],
                log_path=log_path,
            )
            self._build_trial_package(task=task, log_path=log_path)

            detail = "host build completed"
            app_bin = self._host_app_binary()
            if app_bin and self._host_desktop_app_launch_enabled():
                detail += f"; app binary: {app_bin}"
            self._emit_state("task_finish", task=task, status="success", detail=detail)
            return TaskResult(task, "success", detail, log_path)
        except Exception as exc:  # noqa: BLE001
            detail = str(exc)
            self._emit_state("task_finish", task=task, status="failed", detail=detail)
            return TaskResult(task, "failed", detail, log_path)

    def run(self, tasks: Sequence[str]) -> List[TaskResult]:
        results: List[TaskResult] = []
        for task in tasks:
            if task != TASK_HOST:
                results.append(TaskResult(task, "failed", f"Unsupported task: {task}", self.logs_dir / f"{task}.log"))
                continue
            results.append(self.task_host())
        return results


def parse_args() -> argparse.Namespace:
    repo_root = Path(__file__).resolve().parents[1]
    dev_env_profile = _load_dev_env_profile(repo_root / "build" / "dev-env" / "dev_env.json")
    system_name = platform.system()
    home = _expand("~")
    qt_root = _expand(_pick_setting("QT_VERSION_ROOT", dev_env_profile, str(_default_qt_version_root(home))))
    qt_host_default = _default_qt_host_prefix(system_name, qt_root)
    lvrs_default = _default_lvrs_prefix(system_name, home)
    lvrs_source_default = _default_lvrs_source_dir(home, repo_root)

    parser = argparse.ArgumentParser(description="Build and run WhatSon on the current desktop host.")
    parser.add_argument("--root", default=str(repo_root), help="Repository root path.")
    parser.add_argument("--logs-dir", default=str(repo_root / "automation-logs"))
    parser.add_argument("--tasks", default=TASK_HOST, help="Comma-separated tasks. Only host is supported.")
    parser.add_argument(
        "--jobs",
        type=_positive_int_arg,
        default=_default_build_jobs(),
        help="Maximum number of native build jobs to use (default: %(default)s).",
    )
    mode_group = parser.add_mutually_exclusive_group()
    mode_group.add_argument("--sequential", action="store_true", help="Run selected tasks sequentially.")
    mode_group.add_argument("--parallel", action="store_true", help="Accepted for compatibility; host still runs alone.")
    parser.add_argument("--no-host-run", action="store_true", help="Skip launching the host desktop app.")
    parser.add_argument("--host-build-dir", default=str(repo_root / "build"))
    parser.add_argument("--trial-build-dir", default=str(repo_root / "build-trial"))
    parser.add_argument("--qt-version-root", default=str(qt_root))
    parser.add_argument(
        "--qt-host-prefix",
        default=str(_expand(_pick_setting("QT_HOST_PREFIX", dev_env_profile, str(qt_host_default)))),
    )
    parser.add_argument(
        "--lvrs-prefix",
        default=str(_expand(_pick_setting("LVRS_PREFIX", dev_env_profile, str(lvrs_default)))),
    )
    parser.add_argument("--lvrs-dir", default=_pick_setting("LVRS_DIR", dev_env_profile))
    parser.add_argument("--lvrs-source-dir", default=str(lvrs_source_default))
    args = parser.parse_args()
    if not args.sequential and not args.parallel:
        args.sequential = True
    return args


if __name__ == "__main__":
    parsed_args = parse_args()
    selected_tasks = [item.strip() for item in parsed_args.tasks.split(",") if item.strip()]
    invalid_tasks = [task for task in selected_tasks if task not in ALL_TASKS]
    if invalid_tasks:
        print(f"Unsupported task(s): {', '.join(invalid_tasks)}", file=sys.stderr)
        sys.exit(2)
    runner = BuildAll(parsed_args)
    task_results = runner.run(selected_tasks or [TASK_HOST])
    sys.exit(1 if any(result.status == "failed" for result in task_results) else 0)
