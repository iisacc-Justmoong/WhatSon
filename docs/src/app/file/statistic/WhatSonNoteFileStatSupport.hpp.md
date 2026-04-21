# `src/app/models/file/statistic/WhatSonNoteFileStatSupport.hpp`

## Responsibility

`WhatSonNoteFileStatSupport` exposes the note-header statistic synchronization helpers used by
filesystem persistence and note-open tracking.

## Public API

- `extractBacklinkTargets(...)`: parses the supported backlink syntaxes from note source/body text.
- `applyBodyDerivedStatistics(...)`: recomputes counters that depend only on the current note body/header payload and
  deliberately preserves the last known `backlinkByCount`.
- `applyTrackedStatistics(...)`: recalculates the current note's `fileStat` counters in memory.
- `incrementOpenCountForNoteHeader(...)`: rewrites only the persisted `.wsnhead` open-count metadata for note-selection
  tracking without rescanning hub `.wsnbody` files. The helper also refreshes the persisted `lastOpenedAt` timestamp.
- `refreshTrackedStatisticsForNote(...)`: reloads a persisted note, refreshes its counters, and can
  optionally increment `openCount` and `lastOpenedAt`.
- `refreshTrackedStatisticsForNoteId(...)`: resolves a note inside the current hub and refreshes its
  counters without needing the caller to know that note's directory path.
