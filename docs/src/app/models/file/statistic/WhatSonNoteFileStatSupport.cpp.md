# `src/app/models/file/statistic/WhatSonNoteFileStatSupport.cpp`

## Responsibility

This helper owns the derived `.wsnhead <fileStat>` rules.

## Derived Counters

- Header-derived:
  - `totalFolders`
  - `totalTags`
- Body-derived:
  - `letterCount`
  - `wordCount`
  - `sentenceCount`
  - `paragraphCount`
  - `spaceCount`
  - `indentCount`
  - `lineCount`
  - `backlinkToCount`
  - `includedResourceCount`
- Hub-derived:
  - `backlinkByCount`

## Tracking Rules

- `applyBodyDerivedStatistics(...)` is the cheap path. It refreshes only counters that can be derived from the current
  note's own body/header payload and leaves the previously known incoming-backlink count untouched.
- `incrementOpenCountForNoteHeader(...)` is now the cheap note-selection path. It rewrites only `.wsnhead` metadata
  and skips both local body parsing and hub-wide `.wsnbody` backlink scans.
- Both the cheap header-only path and the full `refreshTrackedStatisticsForNote(..., true)` path now stamp
  `lastOpenedAt` with the current UTC ISO timestamp whenever they advance `openCount`.
- `openCount` is still incremented through `refreshTrackedStatisticsForNote(..., true)` when a caller explicitly wants
  the full tracked-stat refresh.
- The helper intentionally does not mutate `modifiedCount`; write paths own that counter.
- Incoming backlink counts are resolved by scanning other `.wsnbody` files under the enclosing
  `.wshub` package and skipping hidden staged-delete directories.
- The save-path split therefore becomes:
  - file store hot path: rewrite local body-derived stats immediately
  - higher-level coordinator/controller path: pay the hub scan later when `backlinkByCount` must be refreshed
