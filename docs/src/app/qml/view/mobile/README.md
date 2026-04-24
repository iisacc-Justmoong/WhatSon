# `src/app/qml/view/mobile`

## Status
- Directory mirror generated from the current `src` tree.
- This file is the entry point for the detailed documentation pass of this directory.

## Scope
- Mirrored source directory: `src/app/qml/view/mobile`
- Child directories: 1
- Child files: 1

## Child Directories
- `pages`

## Child Files
- `MobilePageScaffold.qml`

## Recent Notes
- The mobile routed workspace now explicitly repairs committed detail-page back navigation so the restore target remains the note editor instead of falling through to the hierarchy page.
- The same mobile shell now also routes explicit back dismissal through canonical page targets (`detail -> editor ->
  note-list -> hierarchy`) instead of issuing a raw `router.back()` from the note editor.

## Intended Detailed Sections
- Module responsibilities and architectural layer
- Internal submodule boundaries
- Cross-directory dependencies
- Runtime ownership and lifecycle rules
- Testing strategy and coverage map
- Known hotspots and refactor priorities
