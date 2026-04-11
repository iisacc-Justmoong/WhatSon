# `src/app/file/validator`

## Status
- Directory mirror generated from the current `src` tree.
- This file is the entry point for the detailed documentation pass of this directory.

## Scope
- Mirrored source directory: `src/app/file/validator`
- Child directories: 0
- Child files: 8

## Child Directories
- No child directories.

## Child Files
- `WhatSonHubStructureValidator.cpp`
- `WhatSonHubStructureValidator.hpp`
- `WhatSonLibraryIndexIntegrityValidator.cpp`
- `WhatSonLibraryIndexIntegrityValidator.hpp`
- `WhatSonNoteStorageValidator.cpp`
- `WhatSonNoteStorageValidator.hpp`
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
- `WhatSonStructuredTagLinter` owns proprietary body-tag lint/canonicalization for `break`, `agenda`, `task`, and `callout`.
- The validator layer now serves both filesystem package normalization and note-body structured-tag normalization.
