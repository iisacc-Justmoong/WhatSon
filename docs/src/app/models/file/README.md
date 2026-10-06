# `src/app/models/file`

## Status
- Directory mirror generated from the current `src` tree.
- This file is the entry point for the detailed documentation pass of this directory.

## Scope
- Mirrored source directory: `src/app/models/file`
- Child directories: 11
- Child files: 1

## Child Directories
- `IO`
- `conflict`
- `diff`
- `export`
- `hub`
- `import`
- `note`
- `statistic`
- `sync`
- `validator`
- `viewer`

## Child Files
- `WhatSonDebugTrace.hpp`

## Intended Detailed Sections
- Module responsibilities and architectural layer
- Internal submodule boundaries
- Cross-directory dependencies
- Runtime ownership and lifecycle rules
- Testing strategy and coverage map
- Known hotspots and refactor priorities

## Migration Note
- The file domain now lives under `src/app/models/file`; build/test include paths and mirrored docs must resolve the
  model-domain location instead of the retired `src/app/file` root.
- Hierarchy model/controller code has moved out to `src/app/models/hierarchy`; this file shard now owns persistent
  hub/note/resource storage and related IO helpers, not hierarchy controller composition.

<a id="한국어"></a>

## Korean

This section is a bottom summary for checking the above  README  content in Korean.

- Target:  ``src/app/models/file``  ( `docs/src/app/models/file/README.md` )
- Location:  `docs/src/app/models/file`
- Role: This file describes the structure, responsibility, operating rules, and validation criteria of the corresponding directory or module.
- Criteria: File path, command,  API  name, and detailed change history are maintained based on the original English text.
- On Change: If the above English text is modified, this Korean lower section is also updated to the latest state.
