# `src/app/runtime/scheduler`

## Status
- Directory mirror generated from the current `src` tree.
- This file is the entry point for the detailed documentation pass of this directory.

## Scope
- Mirrored source directory: `src/app/runtime/scheduler`
- Child directories: 0
- Child files: 6

## Child Directories
- No child directories.

## Child Files
- `WhatSonAsyncScheduler.cpp`
- `WhatSonAsyncScheduler.hpp`
- `WhatSonCronExpression.cpp`
- `WhatSonCronExpression.hpp`
- `WhatSonUnixTimeAnalyzer.cpp`
- `WhatSonUnixTimeAnalyzer.hpp`

## Intended Detailed Sections
- Module responsibilities and architectural layer
- Internal submodule boundaries
- Cross-directory dependencies
- Runtime ownership and lifecycle rules
- Testing strategy and coverage map
- Known hotspots and refactor priorities

## Current Notes
- Automated C++ regression coverage now lives in `test/cpp/whatson_cpp_regression_tests.cpp`, locking cron parsing,
  minute-level deduplication, interval trigger progression, unix-time analysis, and scheduler hook state for
  `WhatSonCronExpression`, `WhatSonUnixTimeAnalyzer`, and `WhatSonAsyncScheduler`.
