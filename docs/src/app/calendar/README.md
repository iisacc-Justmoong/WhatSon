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
- Calendar note projection now has two refresh sources: the live library runtime snapshot for startup/library flows and
  `.wshub` disk reindexing for fallback mutation flows outside the library viewmodel.
- Calendar query surfaces also have a live-provider fallback so note lifecycle items can still be resolved while the
  explicit projected cache is empty.
- Each projected note now resolves to a single calendar item anchored to the more recent of `createdAt` and
  `lastModifiedAt`, and its chip label reuses the same top-line preview text rule as the library note list.
