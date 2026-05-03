# `src/app/models/file/hierarchy/library`

## Status
- Directory mirror generated from the current `src` tree.
- This file is the entry point for the detailed documentation pass of this directory.

## Scope
- Mirrored source directory: `src/app/models/file/hierarchy/library`
- Child directories: 0
- Child files: 18

## Child Directories
- No child directories.

## Child Files
- `LibraryAll.cpp`
- `LibraryAll.hpp`
- `LibraryDraft.cpp`
- `LibraryDraft.hpp`
- `LibraryNoteRecord.hpp`
- `LibraryNotePreviewText.hpp`
- `LibraryToday.cpp`
- `LibraryToday.hpp`
- `WhatSonLibraryIndexedState.cpp`
- `WhatSonLibraryIndexedState.hpp`
- `WhatSonLibraryFolderHierarchyMutationService.cpp`
- `WhatSonLibraryFolderHierarchyMutationService.hpp`
- `WhatSonLibraryHierarchyCreator.cpp`
- `WhatSonLibraryHierarchyCreator.hpp`
- `WhatSonLibraryHierarchyParser.cpp`
- `WhatSonLibraryHierarchyParser.hpp`
- `WhatSonLibraryHierarchyStore.cpp`
- `WhatSonLibraryHierarchyStore.hpp`

## Intended Detailed Sections
- Module responsibilities and architectural layer
- Internal submodule boundaries
- Cross-directory dependencies
- Runtime ownership and lifecycle rules
- Testing strategy and coverage map
- Known hotspots and refactor priorities

## Notes
- `LibraryNoteRecord` now exposes structural equality so incremental mutation layers can suppress no-op updates before
  they fan out into note-list/calendar rebuilds.
- `LibraryAll`, `LibraryDraft`, and `LibraryToday` now all support single-note upsert/remove operations, and
  `WhatSonLibraryIndexedState` uses those paths to keep canonical and derived buckets synchronized without replacing
  the whole note snapshot on every local edit.
- `LibraryNotePreviewText.hpp` is the shared preview-text authority for library note cards and calendar note chips, so
  compact note renderers do not drift into separate headline rules.

## 한국어

이 섹션은 위 README 내용을 한국어로 확인하기 위한 하단 요약이다.

- 대상: ``src/app/models/file/hierarchy/library`` (`docs/src/app/models/file/hierarchy/library/README.md`)
- 위치: `docs/src/app/models/file/hierarchy/library`
- 역할: 이 파일은 해당 디렉터리나 모듈의 구조, 책임, 운영 규칙, 검증 기준을 설명한다.
- 기준: 파일 경로, 명령, API 이름, 세부 변경 이력은 위 영어 본문을 원문 기준으로 유지한다.
- 변경 시: 위 영어 본문을 수정하면 이 한국어 하단 섹션도 함께 최신 상태로 맞춘다.
