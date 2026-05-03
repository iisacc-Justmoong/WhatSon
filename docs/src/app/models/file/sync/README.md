# `src/app/models/file/sync`

## Status
- Directory mirror generated from the current `src` tree.
- This file is the entry point for the detailed documentation pass of this directory.

## Scope
- Mirrored source directory: `src/app/models/file/sync`
- Child directories: 0
- Child files: 2

## Child Directories
- No child directories.

## Child Files
- `WhatSonHubSyncController.cpp`
- `WhatSonHubSyncController.hpp`

## Current Notes

- `WhatSonHubSyncController` was moved from `src/app/sync` into this directory so sync responsibilities are now
  fully consolidated under `file/sync`.
- Runtime hub synchronization remains the responsibility of this directory.
- Note editor persistence is no longer part of `file/sync`; it is owned by
  `src/app/models/editor/persistence/ContentsEditorPersistenceController`.

## Tests

- Automated test files are not currently present in this repository.
- Regression checklist:
  - Startup/runtime wiring must still resolve `WhatSonHubSyncController` through the new `file/sync` include path.
  - Hub watcher debounce and local-mutation acknowledge flow must remain unchanged after the directory move.
  - Editor persistence tests must continue to reject any new dependency on the removed editor-persistence path under
    `src/app/models/file/sync`.

## Intended Detailed Sections
- Module responsibilities and architectural layer
- Internal submodule boundaries
- Cross-directory dependencies
- Runtime ownership and lifecycle rules
- Testing strategy and coverage map
- Known hotspots and refactor priorities

## 한국어

이 섹션은 위 README 내용을 한국어로 확인하기 위한 하단 요약이다.

- 대상: ``src/app/models/file/sync`` (`docs/src/app/models/file/sync/README.md`)
- 위치: `docs/src/app/models/file/sync`
- 역할: 이 파일은 해당 디렉터리나 모듈의 구조, 책임, 운영 규칙, 검증 기준을 설명한다.
- 기준: 파일 경로, 명령, API 이름, 세부 변경 이력은 위 영어 본문을 원문 기준으로 유지한다.
- 변경 시: 위 영어 본문을 수정하면 이 한국어 하단 섹션도 함께 최신 상태로 맞춘다.
