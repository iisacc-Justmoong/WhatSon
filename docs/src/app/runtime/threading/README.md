# `src/app/runtime/threading`

## Status
- Directory mirror generated from the current `src` tree.
- This file is the entry point for the detailed documentation pass of this directory.

## Scope
- Mirrored source directory: `src/app/runtime/threading`
- Child directories: 0
- Child files: 4

## Child Directories
- No child directories.

## Child Files
- `WhatSonRuntimeDomainSnapshots.cpp`
- `WhatSonRuntimeDomainSnapshots.hpp`
- `WhatSonRuntimeParallelLoader.cpp`
- `WhatSonRuntimeParallelLoader.hpp`

## Intended Detailed Sections
- Module responsibilities and architectural layer
- Internal submodule boundaries
- Cross-directory dependencies
- Runtime ownership and lifecycle rules
- Testing strategy and coverage map
- Known hotspots and refactor priorities

## Recent Notes
- Startup/reload snapshot application is now all-or-nothing across the requested domain set.
- LVRS `BootstrapParallel` now owns the worker pool for requested domain loads. Live controllers and the shared
  `WhatSonHubRuntimeStore` are still updated only after the entire request succeeds.
- Persisted startup no longer requests any runtime domains before the workspace root is visible. `main.cpp` schedules
  the normal full runtime load as an LVRS `QmlAppLifecycleStage::AfterFirstIdle` bootstrap task.

## 한국어

이 섹션은 위 README 내용을 한국어로 확인하기 위한 하단 요약이다.

- 대상: ``src/app/runtime/threading`` (`docs/src/app/runtime/threading/README.md`)
- 위치: `docs/src/app/runtime/threading`
- 역할: 이 파일은 해당 디렉터리나 모듈의 구조, 책임, 운영 규칙, 검증 기준을 설명한다.
- 기준: 파일 경로, 명령, API 이름, 세부 변경 이력은 위 영어 본문을 원문 기준으로 유지한다.
- 변경 시: 위 영어 본문을 수정하면 이 한국어 하단 섹션도 함께 최신 상태로 맞춘다.
