#!/usr/bin/env python3
from __future__ import annotations

import argparse
import json
import os
import shlex
import subprocess
import sys
import time
from pathlib import Path
from typing import Dict, List, Optional, Sequence

from build_platform_runner import _path_state


class SmokeError(RuntimeError):
    pass


def _expand(value: str | Path) -> Path:
    return Path(os.path.expandvars(os.path.expanduser(str(value)))).resolve()


def _quote(cmd: Sequence[str]) -> str:
    return " ".join(shlex.quote(str(part)) for part in cmd)


def _timestamp_now() -> str:
    return time.strftime("%Y-%m-%dT%H:%M:%S%z", time.localtime())


class RuntimeSmokeMatrix:
    def __init__(self, args: argparse.Namespace) -> None:
        self.root = _expand(args.root)
        self.tasks = [task.strip() for task in args.tasks.split(",") if task.strip()]
        self.jobs = args.jobs
        self.logs_dir = _expand(args.logs_dir)
        self.artifacts_dir = _expand(args.artifacts_dir)
        self.logs_dir.mkdir(parents=True, exist_ok=True)
        self.artifacts_dir.mkdir(parents=True, exist_ok=True)

    def _emit_state(self, event: str, **fields) -> None:
        payload: Dict[str, object] = {
            "ts": _timestamp_now(),
            "script": "runtime_smoke_matrix",
            "event": event,
            "root": str(self.root),
        }
        payload.update({key: value for key, value in fields.items() if value is not None})
        print(f"[state] {json.dumps(payload, ensure_ascii=False, sort_keys=True)}", flush=True)

    def _run(self, *, name: str, cmd: Sequence[str], check: bool = True) -> subprocess.CompletedProcess[str]:
        log_path = self.logs_dir / f"{name}.log"
        cmd_list = [str(part) for part in cmd]
        self._emit_state("command_start", name=name, cmd=cmd_list, cmd_text=_quote(cmd_list), log_path=_path_state(log_path))
        with log_path.open("w", encoding="utf-8") as fp:
            fp.write(f"$ {_quote(cmd_list)}\n")
            process = subprocess.run(
                cmd_list,
                cwd=str(self.root),
                stdout=fp,
                stderr=subprocess.STDOUT,
                text=True,
                check=False,
            )
            fp.write(f"[exit] {process.returncode}\n")
        self._emit_state("command_finish", name=name, returncode=process.returncode, log_path=_path_state(log_path))
        if check and process.returncode != 0:
            raise SmokeError(f"{name} failed ({process.returncode}): {_quote(cmd_list)}")
        return process

    def host_smoke(self) -> None:
        if "host" not in self.tasks:
            return
        self._emit_state("phase_start", phase="host_smoke")
        self._run(
            name="host_build_all",
            cmd=[
                sys.executable,
                str(self.root / "scripts" / "build_all.py"),
                "--root",
                str(self.root),
                "--tasks",
                "host",
                "--jobs",
                str(self.jobs),
                "--no-host-run",
            ],
        )
        self._emit_state("phase_finish", phase="host_smoke", status="success")

    def run(self) -> int:
        unsupported_tasks = [task for task in self.tasks if task != "host"]
        if unsupported_tasks:
            raise SmokeError(f"Unsupported runtime smoke task(s): {', '.join(unsupported_tasks)}")

        self._emit_state(
            "script_start",
            tasks=self.tasks,
            jobs=self.jobs,
            logs_dir=_path_state(self.logs_dir),
            artifacts_dir=_path_state(self.artifacts_dir),
        )
        self.host_smoke()
        self._emit_state("script_finish", status="success")
        return 0


def parse_args() -> argparse.Namespace:
    repo_root = Path(__file__).resolve().parents[1]
    parser = argparse.ArgumentParser(description="Desktop host runtime smoke matrix for WhatSon.")
    parser.add_argument("--root", default=str(repo_root))
    parser.add_argument("--tasks", default="host", help="Comma-separated tasks. Only host is supported.")
    parser.add_argument("--logs-dir", default=str(repo_root / "automation-logs" / "runtime-smoke"))
    parser.add_argument("--artifacts-dir", default=str(repo_root / "build" / "runtime-smoke"))
    parser.add_argument("--jobs", type=int, default=max(1, min(os.cpu_count() or 1, 8)))
    return parser.parse_args()


def main() -> int:
    try:
        return RuntimeSmokeMatrix(parse_args()).run()
    except SmokeError as exc:
        print(f"[runtime-matrix] failed: {exc}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main())
