#!/usr/bin/env python3
from __future__ import annotations

import sys

from build_platform_runner import BuildAll, TASK_IOS, _path_state, emit_state, parse_args


def main() -> int:
    args = parse_args()
    args.tasks = TASK_IOS
    args.sequential = True

    runner = BuildAll(args)
    emit_state(
        "build_ios",
        "script_start",
        task=TASK_IOS,
        root=_path_state(runner.root),
        logs_dir=_path_state(runner.logs_dir),
        project_dir=_path_state(runner.ios_project_dir),
        jobs=runner.build_jobs,
        ios_bundle_id=runner.ios_bundle_id,
        ios_device=runner.ios_device,
    )
    print(f"[build_ios] root={runner.root}", flush=True)
    print("[build_ios] mode=xcodeproj-config", flush=True)

    result = runner.run([TASK_IOS])[0]
    emit_state(
        "build_ios",
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
