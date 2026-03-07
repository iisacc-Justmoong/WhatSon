#!/usr/bin/env python3
from __future__ import annotations

import sys

from build_platform_runner import BuildAll, TASK_HOST, _path_state, emit_state, parse_args


def main() -> int:
    args = parse_args()
    args.tasks = TASK_HOST
    args.sequential = True

    runner = BuildAll(args)
    emit_state(
        "build_host",
        "script_start",
        task=TASK_HOST,
        root=_path_state(runner.root),
        logs_dir=_path_state(runner.logs_dir),
        build_dir=_path_state(runner.host_build_dir),
        jobs=runner.build_jobs,
        no_host_run=runner.no_host_run,
    )
    print(f"[build_host] root={runner.root}", flush=True)

    result = runner.run([TASK_HOST])[0]
    emit_state(
        "build_host",
        "script_finish",
        task=result.name,
        status=result.status,
        detail=result.detail,
        log_path=_path_state(result.log_path),
    )
    print(f"[{result.name}] {result.status}: {result.detail}", flush=True)
    print(f"[{result.name}] log: {result.log_path}", flush=True)
    return 0 if result.status != "failed" else 1


if __name__ == "__main__":
    sys.exit(main())
