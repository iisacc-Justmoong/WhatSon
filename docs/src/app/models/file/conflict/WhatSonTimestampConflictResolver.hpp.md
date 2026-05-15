# `src/app/models/file/conflict/WhatSonTimestampConflictResolver.hpp`

## Responsibility

Declares the timestamp-based note body conflict resolver.

## Contract

- Accepts a base pull timestamp, current filesystem timestamp, incoming editor timestamp, and both body candidates.
- Reports whether a conflict was detected and which side won.
- Uses `incoming` as the default winner when the filesystem did not advance after the base pull.
- Uses `filesystem` when both sides changed but the filesystem timestamp is newer than the incoming timestamp.
- Exposes `isTimestampNewer(...)` for strict read-side freshness checks that should not perform a full body merge.

## Boundary

- The resolver does not parse `.wsnbody`, mutate `.wsnhead`, or persist files.
- File IO and version diff capture remain in `WhatSonLocalNoteFileStore`.
