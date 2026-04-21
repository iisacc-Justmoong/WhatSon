# `src/app/models/file`

## Status
- Directory mirror generated from the current `src` tree.
- This file is the entry point for the detailed documentation pass of this directory.

## Scope
- Mirrored source directory: `src/app/models/file`
- Child directories: 12
- Child files: 1

## Child Directories
- `IO`
- `conflict`
- `diff`
- `export`
- `hierarchy`
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
