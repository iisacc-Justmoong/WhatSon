# `src/app/models/file/statistic/WhatSonNoteFileStatSupport.cpp`

## Responsibility

This helper owns the remaining header-only note-open statistic rules.

## Header Counters

- `openCount`
- `lastOpenedAt`

## Tracking Rules

- `incrementOpenCountForNoteHeader(...)` is now the cheap note-selection path. It rewrites only `.wsnhead` metadata
  and skips body parsing and hub-wide backlink scans.
- Both the cheap header-only path and the full `refreshTrackedStatisticsForNote(..., true)` path now stamp
  `lastOpenedAt` with the current UTC ISO timestamp whenever they advance `openCount`.
- `openCount` is still incremented through `refreshTrackedStatisticsForNote(..., true)` when a caller explicitly wants
  the tracked-stat refresh.
- The helper intentionally does not mutate `modifiedCount`; write paths own that counter.
