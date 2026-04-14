# `src/app/file/validator`

## Status
- Directory mirror generated from the current `src` tree.
- This file is the entry point for the detailed documentation pass of this directory.

## Scope
- Mirrored source directory: `src/app/file/validator`
- Child directories: 0
- Child files: 10

## Child Directories
- No child directories.

## Child Files
- `WhatSonHubStructureValidator.cpp`
- `WhatSonHubStructureValidator.hpp`
- `WhatSonLibraryIndexIntegrityValidator.cpp`
- `WhatSonLibraryIndexIntegrityValidator.hpp`
- `WhatSonNoteStorageValidator.cpp`
- `WhatSonNoteStorageValidator.hpp`
- `ContentsStructuredTagValidator.cpp`
- `ContentsStructuredTagValidator.hpp`
- `WhatSonStructuredTagLinter.cpp`
- `WhatSonStructuredTagLinter.hpp`

## Intended Detailed Sections
- Module responsibilities and architectural layer
- Internal submodule boundaries
- Cross-directory dependencies
- Runtime ownership and lifecycle rules
- Testing strategy and coverage map
- Known hotspots and refactor priorities

## Current Domain Notes
- `WhatSonStructuredTagLinter` owns proprietary body-tag lint/canonicalization for `break`, `agenda`, `task`, and
  `callout`, and also validates that supported `.wsnbody` semantic/body tags still produce one well-formed XML
  document projection.
- That XML projection covers body/resource semantics such as `paragraph`, heading-style semantic tags
  (`title`/`subTitle`/`eventTitle`), `eventDescription`, transparent `event` wrappers, `next`, hashtag tags, and
  inline style aliases.
- `ContentsStructuredTagValidator` remains available as an opt-in direct-correction helper, but editor hosts no longer
  auto-wire parser/renderer suggestions into direct note writes during ordinary note-open or typing.
- The validator layer now serves both filesystem package normalization and note-body structured-tag normalization.
