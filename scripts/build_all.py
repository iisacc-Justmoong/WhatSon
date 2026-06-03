#!/usr/bin/env python3
from __future__ import annotations

import argparse
import shlex
import subprocess
import sys
from dataclasses import dataclass
from pathlib import Path
from typing import List, Sequence, Tuple

from build_platform_runner import (
    TASK_HOST,
    _default_build_jobs,
    _path_state,
    _positive_int_arg,
    emit_state,
)

ALL_TASKS: Tuple[str, ...] = (TASK_HOST,)


@dataclass
class TaskResult:
    name: str
    status: str
    detail: str


def _quote(cmd: Sequence[str]) -> str:
    return " ".join(shlex.quote(str(item)) for item in cmd)


def _task_script(root: Path, task: str) -> Path:
    if task != TASK_HOST:
        raise RuntimeError(f"Unsupported task: {task}")
    script = root / "scripts" / "build_host.py"
    if not script.exists():
        raise RuntimeError(f"Task script not found for '{task}': {script}")
    return script


def _run_task(*, root: Path, task: str, passthrough_args: Sequence[str], jobs: int) -> TaskResult:
    try:
        script_path = _task_script(root, task)
        cmd = [sys.executable, str(script_path), "--root", str(root), "--jobs", str(jobs), *passthrough_args]
        cmd_text = _quote(cmd)
        emit_state(
            "build_all",
            "child_exec_start",
            task=task,
            jobs=jobs,
            root=_path_state(root),
            script_path=_path_state(script_path),
            cmd=cmd,
            cmd_text=cmd_text,
            passthrough_args=list(passthrough_args),
        )
        print(f"[build_all:{task}] exec: {cmd_text}", flush=True)
        proc = subprocess.run(cmd, cwd=str(root), check=False)
        emit_state(
            "build_all",
            "child_exec_finish",
            task=task,
            jobs=jobs,
            cmd=cmd,
            cmd_text=cmd_text,
            returncode=proc.returncode,
        )
        if proc.returncode == 0:
            return TaskResult(task, "success", "completed")
        return TaskResult(task, "failed", f"exit={proc.returncode}")
    except Exception as exc:  # noqa: BLE001
        emit_state("build_all", "child_exec_finish", task=task, status="failed", detail=str(exc))
        return TaskResult(task, "failed", str(exc))


def _parse_args() -> tuple[argparse.Namespace, list[str]]:
    repo_root = Path(__file__).resolve().parents[1]
    parser = argparse.ArgumentParser(description="Desktop host build orchestrator.")
    parser.add_argument("--root", default=str(repo_root), help="Repository root path.")
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
    args, passthrough_args = parser.parse_known_args()
    if not args.sequential and not args.parallel:
        args.sequential = True
    return args, passthrough_args


def main() -> int:
    args, passthrough_args = _parse_args()
    root = Path(args.root).expanduser().resolve()
    requested = [item.strip() for item in args.tasks.split(",") if item.strip()]
    tasks = [task for task in requested if task in ALL_TASKS] or [TASK_HOST]
    invalid_tasks = [task for task in requested if task not in ALL_TASKS]
    if invalid_tasks:
        emit_state("build_all", "invalid_tasks", invalid_tasks=invalid_tasks)
        print(f"[build_all] fatal: unsupported task(s): {', '.join(invalid_tasks)}", flush=True)
        return 2

    emit_state(
        "build_all",
        "script_start",
        root=_path_state(root),
        requested_tasks=requested,
        selected_tasks=tasks,
        mode="host",
        jobs=args.jobs,
        passthrough_args=list(passthrough_args),
    )
    print(f"[build_all] root={root}", flush=True)
    print(f"[build_all] tasks={','.join(tasks)}", flush=True)
    print(f"[build_all] jobs={args.jobs}", flush=True)

    results: List[TaskResult] = [
        _run_task(root=root, task=task, passthrough_args=passthrough_args, jobs=args.jobs)
        for task in tasks
    ]

    has_failure = False
    for result in results:
        emit_state("build_all", "task_result", task=result.name, status=result.status, detail=result.detail)
        print(f"[build_all:{result.name}] {result.status}: {result.detail}", flush=True)
        if result.status == "failed":
            has_failure = True

    emit_state(
        "build_all",
        "script_finish",
        status="failed" if has_failure else "success",
        results=[{"task": result.name, "status": result.status, "detail": result.detail} for result in results],
    )
    return 1 if has_failure else 0


if __name__ == "__main__":
    sys.exit(main())
