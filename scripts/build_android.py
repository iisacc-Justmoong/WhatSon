#!/usr/bin/env python3
from __future__ import annotations

import sys

from build_platform_runner import BuildAll, TASK_ANDROID, _path_state, emit_state, parse_args


def main() -> int:
    args = parse_args()
    args.tasks = TASK_ANDROID
    args.sequential = True

    runner = BuildAll(args)
    emit_state(
        "build_android",
        "script_start",
        task=TASK_ANDROID,
        root=_path_state(runner.root),
        logs_dir=_path_state(runner.logs_dir),
        build_dir=_path_state(runner.android_build_dir),
        studio_dir=_path_state(runner.android_studio_dir),
        jobs=runner.build_jobs,
        android_package=runner.android_package,
        android_allow_emulator=runner.android_allow_emulator,
    )
    print(f"[build_android] root={runner.root}", flush=True)

    result = runner.run([TASK_ANDROID])[0]
    emit_state(
        "build_android",
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
