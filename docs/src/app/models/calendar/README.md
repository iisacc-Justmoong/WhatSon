# `src/app/calendar`

## Status
- Directory mirror generated from the current `src` tree.
- This file is the entry point for the detailed documentation pass of this directory.

## Scope
- Mirrored source directory: `src/app/calendar`
- Child directories: 0
- Child files: 4

## Child Directories
- No child directories.

## Child Files
- `CalendarBoardStore.cpp`
- `CalendarBoardStore.hpp`
- `SystemCalendarStore.cpp`
- `SystemCalendarStore.hpp`

## Intended Detailed Sections
- Module responsibilities and architectural layer
- Internal submodule boundaries
- Cross-directory dependencies
- Runtime ownership and lifecycle rules
- Testing strategy and coverage map
- Known hotspots and refactor priorities

## Notes
- `CalendarBoardStore` owns both user-authored calendar board entries and read-only note lifecycle projections derived
  from the current hub's library index.
- The store now keeps date-keyed entry/count indexes for both manual board items and projected note items, which
  lowers the rebuild/query cost of month/year calendar controllers.
- Calendar note projection now has two refresh sources: the live library runtime snapshot for startup/library flows and
  `.wshub` disk reindexing for fallback mutation flows outside the library controller.
- Library-originated note mutations can now update projected calendar mounts through single-note upsert/remove APIs
  instead of replacing the entire projection on every note save.
- Calendar query surfaces also have a live-provider fallback so note lifecycle items can still be resolved while the
  explicit projected cache is empty.
- Each projected note now resolves to a single calendar item anchored to the more recent of `createdAt` and
  `lastModifiedAt`, and its chip label reuses the same top-line preview text rule as the library note list.

## 한국어

이 섹션은 위 README 내용을 한국어로 확인하기 위한 하단 요약이다.

- 대상: ``src/app/calendar`` (`docs/src/app/models/calendar/README.md`)
- 위치: `docs/src/app/models/calendar`
- 역할: 이 파일은 해당 디렉터리나 모듈의 구조, 책임, 운영 규칙, 검증 기준을 설명한다.
- 기준: 파일 경로, 명령, API 이름, 세부 변경 이력은 위 영어 본문을 원문 기준으로 유지한다.
- 변경 시: 위 영어 본문을 수정하면 이 한국어 하단 섹션도 함께 최신 상태로 맞춘다.
