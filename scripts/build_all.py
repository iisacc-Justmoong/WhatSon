#!/usr/bin/env python3
from __future__ import annotations

import argparse
import os
import shlex
import subprocess
from dataclasses import dataclass
from typing import Dict, List, Sequence, Tuple

import sys
from concurrent.futures import ThreadPoolExecutor, as_completed
from pathlib import Path

TASK_HOST = "host"
TASK_ANDROID = "android"
TASK_IOS = "ios"
ALL_TASKS: Tuple[str, ...] = (TASK_HOST, TASK_ANDROID, TASK_IOS)


@dataclass
class TaskResult:
    name: str
    status: str  # success, failed
    detail: str


def _expand(path: str) -> Path:
    return Path(os.path.expandvars(os.path.expanduser(path))).resolve()


def _quote(cmd: Sequence[str]) -> str:
    return " ".join(shlex.quote(str(item)) for item in cmd)


def _task_script(root: Path, task: str) -> Path:
    script_map = {
        TASK_HOST: root / "scripts" / "build_host.py",
        TASK_ANDROID: root / "scripts" / "build_android.py",
        TASK_IOS: root / "scripts" / "build_ios.py",
    }
    script = script_map[task]
    if not script.exists():
        raise RuntimeError(f"Task script not found for '{task}': {script}")
    return script


def _run_task(*, root: Path, task: str, passthrough_args: Sequence[str]) -> TaskResult:
    try:
        script_path = _task_script(root, task)
        cmd = [sys.executable, str(script_path), "--root", str(root), *passthrough_args]
        print(f"[build_all:{task}] exec: {_quote(cmd)}")
        proc = subprocess.run(cmd, cwd=str(root), check=False)
        if proc.returncode == 0:
            return TaskResult(task, "success", "completed")
        return TaskResult(task, "failed", f"exit={proc.returncode}")
    except Exception as exc:  # noqa: BLE001
        return TaskResult(task, "failed", str(exc))


def _parse_args() -> tuple[argparse.Namespace, list[str]]:
    repo_root = Path(__file__).resolve().parents[1]

    parser = argparse.ArgumentParser(
        description=(
            "Platform-split build orchestrator. "
            "Runs build_host.py, build_android.py, and build_ios.py."
        )
    )
    parser.add_argument("--root", default=str(repo_root), help="Repository root path.")
    parser.add_argument("--tasks", default="host,android,ios", help="Comma-separated tasks: host,android,ios")

    mode_group = parser.add_mutually_exclusive_group()
    mode_group.add_argument(
        "--sequential",
        action="store_true",
        help="Run selected platform scripts sequentially (default).",
    )
    mode_group.add_argument(
        "--parallel",
        action="store_true",
        help="Run selected platform scripts in parallel.",
    )

    args, passthrough_args = parser.parse_known_args()
    if not args.sequential and not args.parallel:
        args.sequential = True
    return args, passthrough_args


def main() -> int:
    args, passthrough_args = _parse_args()
    root = _expand(args.root)

    requested = [item.strip() for item in args.tasks.split(",") if item.strip()]
    tasks = [task for task in requested if task in ALL_TASKS]
    if not tasks:
        print("[build_all] fatal: no valid tasks were selected.")
        return 1

    invalid_tasks = [task for task in requested if task not in ALL_TASKS]
    for invalid in invalid_tasks:
        print(f"[build_all] warning: ignored unknown task '{invalid}'")

    mode_text = "parallel" if args.parallel else "sequential"
    print(f"[build_all] root={root}")
    print(f"[build_all] tasks={','.join(tasks)}")
    print(f"[build_all] mode={mode_text}")

    results: List[TaskResult]
    if args.parallel and len(tasks) > 1:
        result_map: Dict[str, TaskResult] = {}
        with ThreadPoolExecutor(max_workers=len(tasks)) as executor:
            future_map = {
                executor.submit(_run_task, root=root, task=task, passthrough_args=passthrough_args): task
                for task in tasks
            }
            for future in as_completed(future_map):
                task = future_map[future]
                result_map[task] = future.result()
        results = [result_map[task] for task in tasks]
    else:
        results = [
            _run_task(root=root, task=task, passthrough_args=passthrough_args)
            for task in tasks
        ]

    has_failure = False
    for result in results:
        print(f"[build_all:{result.name}] {result.status}: {result.detail}")
        if result.status == "failed":
            has_failure = True

    return 1 if has_failure else 0


if __name__ == "__main__":
    sys.exit(main())
