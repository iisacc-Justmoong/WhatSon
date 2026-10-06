# `src/app/models/file/conflict`

## Status
- Directory mirror generated from the current `src` tree.
- This file is the entry point for the detailed documentation pass of this directory.

## Scope
- Mirrored source directory: `src/app/models/file/conflict`
- Child directories: 0
- Child files: 2

## Child Directories
- No child directories.

## Child Files
- `WhatSonTimestampConflictResolver.cpp`
- `WhatSonTimestampConflictResolver.hpp`

## Current Notes

- `WhatSonTimestampConflictResolver` owns pure timestamp-based body conflict resolution.
- The first supported policy is whole-body last-writer-wins by note timestamps:
  - if the filesystem note has not advanced past the editor pull base timestamp, the incoming editor body wins
  - if both sides changed after the editor pull base timestamp, the newer `lastModified` timestamp wins
- The resolver also exposes strict newer-than comparison for read-side sync, allowing idle editor pulls to ignore
  filesystem bodies whose `lastModified` timestamp is not newer than the currently loaded editor session.
- The resolver does not read or write note packages. Callers supply the base/filesystem/incoming timestamps and decide
  how to persist the chosen body.

## Intended Detailed Sections
- Module responsibilities and architectural layer
- Internal submodule boundaries
- Cross-directory dependencies
- Runtime ownership and lifecycle rules
- Testing strategy and coverage map
- Known hotspots and refactor priorities

<a id="한국어"></a>

## Korean

This section is a bottom summary for checking the above  README  content in Korean.

- Target:  ``src/app/models/file/conflict``  ( `docs/src/app/models/file/conflict/README.md` )
- Location:  `docs/src/app/models/file/conflict`
- Role: This file describes the structure, responsibility, operating rules, and validation criteria of the corresponding directory or module.
- Criteria: File path, command,  API  name, and detailed change history are maintained based on the original English text.
- On Change: If the above English text is modified, this Korean lower section is also updated to the latest state.
