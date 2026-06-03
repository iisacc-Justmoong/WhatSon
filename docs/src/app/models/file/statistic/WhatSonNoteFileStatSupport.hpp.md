# `src/app/models/file/statistic/WhatSonNoteFileStatSupport.hpp`

## Responsibility

`WhatSonNoteFileStatSupport` exposes the note-header statistic synchronization helpers used by note-open tracking.

## Public API

- `incrementOpenCountForNoteHeader(...)`: rewrites only the persisted `.wsnhead` open-count metadata for note-selection
  tracking. The helper also refreshes the persisted `lastOpenedAt` timestamp.
- `refreshTrackedStatisticsForNote(...)`: validates a persisted note header and can optionally increment `openCount`
  and `lastOpenedAt`.
- `refreshTrackedStatisticsForNoteId(...)`: resolves a note inside the current hub and refreshes its
  counters without needing the caller to know that note's directory path.
