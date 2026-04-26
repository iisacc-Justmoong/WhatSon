# `src/app/viewmodel`

## Status
- Directory mirror generated from the current `src` tree.
- This file is the entry point for the detailed documentation pass of this directory.

## Scope
- Mirrored source directory: `src/app/viewmodel`
- Child directories: 10
- Child files: 0

## Boundary

- ViewModels in this tree are C++ objects with one responsibility per class.
- QML files are not registered from `src/app/viewmodel`; QML orchestration surfaces belong under the owning model or
  view directory instead.
- Each ViewModel must expose a signal/slot contract suitable for QML binding while leaving domain mutation and timing
  mechanics in the owning model layer.

## Child Directories
- `calendar`
- `content`
- `detailPanel`
- `editor`
- `hierarchy`
- `navigationbar`
- `onboarding`
- `panel`
- `sidebar`
- `statusbar`

## Child Files
- No direct source files.

## Intended Detailed Sections
- Module responsibilities and architectural layer
- Internal submodule boundaries
- Cross-directory dependencies
- Runtime ownership and lifecycle rules
- Testing strategy and coverage map
- Known hotspots and refactor priorities
