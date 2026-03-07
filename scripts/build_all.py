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

from build_platform_runner import (
    _default_build_jobs,
    _parallel_worker_count,
    _path_state,
    _positive_int_arg,
    _task_job_limits,
    emit_state,
)

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
        emit_state(
            "build_all",
            "child_exec_finish",
            task=task,
            jobs=jobs,
            status="failed",
            detail=str(exc),
        )
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
    parser.add_argument(
        "--jobs",
        type=_positive_int_arg,
        default=_default_build_jobs(),
        help="Maximum number of native build jobs to use across active tasks (default: %(default)s).",
    )

    mode_group = parser.add_mutually_exclusive_group()
    mode_group.add_argument(
        "--sequential",
        action="store_true",
        help="Run selected platform scripts sequentially (default).",
    )
    mode_group.add_argument(
        "--parallel",
        action="store_true",
        help="Run selected platform scripts in parallel and split the build job budget across active tasks.",
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
        emit_state("build_all", "script_finish", status="failed", detail="no valid tasks were selected")
        print("[build_all] fatal: no valid tasks were selected.", flush=True)
        return 1

    invalid_tasks = [task for task in requested if task not in ALL_TASKS]
    for invalid in invalid_tasks:
        print(f"[build_all] warning: ignored unknown task '{invalid}'", flush=True)
    if invalid_tasks:
        emit_state("build_all", "invalid_tasks", invalid_tasks=invalid_tasks)

    mode_text = "parallel" if args.parallel else "sequential"
    emit_state(
        "build_all",
        "script_start",
        root=_path_state(root),
        requested_tasks=requested,
        selected_tasks=tasks,
        invalid_tasks=invalid_tasks or None,
        mode=mode_text,
        jobs=args.jobs,
        passthrough_args=list(passthrough_args),
    )
    print(f"[build_all] root={root}", flush=True)
    print(f"[build_all] tasks={','.join(tasks)}", flush=True)
    print(f"[build_all] mode={mode_text}", flush=True)
    print(f"[build_all] jobs={args.jobs}", flush=True)

    results: List[TaskResult]
    if args.parallel and len(tasks) > 1:
        worker_count = _parallel_worker_count(len(tasks), args.jobs)
        task_jobs = _task_job_limits(tasks, args.jobs)
        emit_state(
            "build_all",
            "job_budget",
            worker_count=worker_count,
            task_job_limits=task_jobs,
        )
        result_map: Dict[str, TaskResult] = {}
        with ThreadPoolExecutor(max_workers=worker_count) as executor:
            future_map = {
                executor.submit(
                    _run_task,
                    root=root,
                    task=task,
                    passthrough_args=passthrough_args,
                    jobs=task_jobs[task],
                ): task
                for task in tasks
            }
            for future in as_completed(future_map):
                task = future_map[future]
                result_map[task] = future.result()
        results = [result_map[task] for task in tasks]
    else:
        emit_state(
            "build_all",
            "job_budget",
            worker_count=1,
            task_job_limits={task: args.jobs for task in tasks},
        )
        results = [
            _run_task(root=root, task=task, passthrough_args=passthrough_args, jobs=args.jobs)
            for task in tasks
        ]

    has_failure = False
    for result in results:
        emit_state(
            "build_all",
            "task_result",
            task=result.name,
            status=result.status,
            detail=result.detail,
        )
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
