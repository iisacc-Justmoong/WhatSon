# `src/app/models/sensor`

## Responsibility
Owns read-side hub inspection objects that derive sensor outputs from the unpacked `.wshub` filesystem layout.

## Scope
- Source directory: `src/app/models/sensor`
- Child files:
  - `MonthlyUnusedNote.hpp`
  - `MonthlyUnusedNote.cpp`
  - `UnusedNoteSensorSupport.hpp`
  - `UnusedNoteSensorSupport.cpp`
  - `UnusedResourcesSensor.hpp`
  - `UnusedResourcesSensor.cpp`
  - `WeeklyUnusedNote.hpp`
  - `WeeklyUnusedNote.cpp`

## Current Contract
- `UnusedNoteSensorSupport` owns the shared hub scan that walks unpacked `.wsnote` packages, parses
  `.wsnhead`, and derives the effective activity timestamp in this order:
  - `lastOpenedAt`
  - `createdAt`
  - `lastModifiedAt`
- `WeeklyUnusedNote` returns note ids whose effective activity timestamp is at least seven days old.
- `MonthlyUnusedNote` returns note ids whose effective activity timestamp is at least one calendar month old.
- Both note sensors surface the richer `unusedNotes` entry list for callers that need note paths, timestamps,
  and the fallback source that decided the inactivity window.
- `UnusedResourcesSensor` scans hub-local `.wsresource` packages and compares them against `<resource ... />`
  embeddings found in note `.wsnbody` documents.
- The sensor returns only packages that are present in the hub but unused by every note.
- The sensor treats RAW note source and package metadata as the source of truth; it does not inspect editor-side DOM
  projections.
