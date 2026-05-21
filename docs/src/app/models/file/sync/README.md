# `src/app/models/file/sync`

## Status
- Directory mirror generated from the current `src` tree.
- This file is the entry point for the detailed documentation pass of this directory.

## Scope
- Mirrored source directory: `src/app/models/file/sync`
- Child directories: 0
- Child files: 13

## Child Directories
- No child directories.

## Child Files
- `WhatSonEditorRawPullController.cpp`
- `WhatSonEditorRawPullController.hpp`
- `WhatSonEditorRawPushController.cpp`
- `WhatSonEditorRawPushController.hpp`
- `WhatSonHubSyncController.cpp`
- `WhatSonHubSyncController.hpp`
- `WhatSonHubSyncObservation.hpp`
- `WhatSonHubSyncObservationBuilder.cpp`
- `WhatSonHubSyncObservationBuilder.hpp`
- `WhatSonHubSyncScheduler.cpp`
- `WhatSonHubSyncScheduler.hpp`
- `WhatSonHubSyncWatcher.cpp`
- `WhatSonHubSyncWatcher.hpp`

## Current Notes

- `WhatSonHubSyncController` was moved from `src/app/sync` into this directory so sync responsibilities are now
  fully consolidated under `file/sync`.
- Runtime hub synchronization remains the responsibility of this directory.
- `WhatSonHubSyncController` is now the facade/orchestrator for mounted hub sync.
- `WhatSonHubSyncObservationBuilder` owns recursive hub inspection, signature hashing, and watch-path discovery.
- `WhatSonHubSyncWatcher` owns `QFileSystemWatcher` registration and watcher path diffs.
- `WhatSonHubSyncScheduler` owns periodic and debounce timers.
- `WhatSonEditorRawPullController` owns note-entry/open RAW pull requests and active-note idle RAW pull scheduling from
  the editor session into the filesystem load queue. Its default idle interval is 5000 ms. The controller stays in
  `file/sync` because note editor sync is bidirectional; concrete `.wsnbody` reads, timestamp winner checks, editor
  projection, and session-file writes still remain in `ContentsNoteManagementCoordinator` and
  `NoteEditorDocumentSession`.
- `WhatSonEditorRawPushController` owns editor-surface-to-RAW push triggers for idle turns, note departure, and editor
  modified-count increments. The controller stays in `file/sync` because it is sync orchestration; concrete
  `.wsnbody` conversion and note-package writes still remain in `NoteEditorDocumentSession` and `file/note`. A pending
  modified-count push has priority over a later idle sync trigger so a stale sync-finished event cannot replace the
  first user deletion snapshot.

## Tests

- `test/cpp/suites/hub_sync_controller_tests.cpp` verifies the split responsibility boundary and observation behavior.
- `test/cpp/suites/editor_raw_pull_controller_tests.cpp` verifies the editor RAW pull request controller.
- `test/cpp/suites/editor_raw_push_controller_tests.cpp` verifies the editor RAW push trigger controller.
- Regression checklist:
  - Startup/runtime wiring must still resolve `WhatSonHubSyncController` through the new `file/sync` include path.
  - Hub watcher debounce and local-mutation acknowledge flow must remain unchanged after the directory move.
  - Editor RAW pull/push orchestration may live under `file/sync`, but RAW parsing/serialization and note-package reads/writes
    must remain outside the sync module.

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
- 역할: 이 파일은 hub sync 모듈의 구조, 책임 분리, 운영 규칙, 검증 기준을 설명한다.
- 기준: 파일 경로, 명령, API 이름, 세부 변경 이력은 위 영어 본문을 원문 기준으로 유지한다.
- 변경 시: 위 영어 본문을 수정하면 이 한국어 하단 섹션도 함께 최신 상태로 맞춘다.
