# `src/app/file/note/WhatSonNoteFileStatSupport.hpp`

## Responsibility

`WhatSonNoteFileStatSupport` exposes the note-header statistic synchronization helpers used by
filesystem persistence and note-open tracking.

## Public API

- `extractBacklinkTargets(...)`: parses the supported backlink syntaxes from note source/body text.
- `applyTrackedStatistics(...)`: recalculates the current note's `fileStat` counters in memory.
- `refreshTrackedStatisticsForNote(...)`: reloads a persisted note, refreshes its counters, and can
  optionally increment `openCount`.
- `refreshTrackedStatisticsForNoteId(...)`: resolves a note inside the current hub and refreshes its
  counters without needing the caller to know that note's directory path.
