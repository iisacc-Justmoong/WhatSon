#!/usr/bin/env python3
from __future__ import annotations

import sys

from build_platform_runner import BuildAll, TASK_ANDROID, parse_args


def main() -> int:
    args = parse_args()
    args.tasks = TASK_ANDROID
    args.sequential = True

    runner = BuildAll(args)
    print(f"[build_android] root={runner.root}")

    result = runner.run([TASK_ANDROID])[0]
    print(f"[{result.name}] {result.status}: {result.detail}")
    print(f"[{result.name}] log: {result.log_path}")
    return 0 if result.status != "failed" else 1


if __name__ == "__main__":
    sys.exit(main())
