# `src/app/models/file/diff/WhatSonNoteVersionDiffBuilder.hpp`

## Role
Declares the in-memory diff builder for note version snapshots.

## Contract
- Builds `WhatSonNoteVersionDiffSegment` from two text payloads and a file label.
- Owns prefix/suffix range calculation and the unified patch text boundary through its implementation file.
- Does not read or write note package files and does not parse `.wsnversion` JSON.

## Tests
- `test/cpp/suites/note_version_store_tests.cpp` covers this helper through snapshot capture and explicit snapshot diff
  flows.
