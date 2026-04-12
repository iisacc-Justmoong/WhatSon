# `src/app/platform/Android`

## Status
- Directory mirror generated from the current `src` tree.
- This file is the entry point for the detailed documentation pass of this directory.

## Scope
- Mirrored source directory: `src/app/platform/Android`
- Child directories: 0
- Child files: 2

## Responsibilities
- Android SAF-backed `.wshub` packages remain the source of truth even though the runtime edits a deterministic
  app-local mounted working copy.
- `WhatSonAndroidStorageBackend` owns three phases:
  - resolving mountable `content://` document-tree selections
  - materializing a local mounted hub under app data for the runtime/editor stack
  - mirroring changed local mounted paths back into the original SAF tree after successful persistence

## Persistence Contract
- Mounted local hub paths are runtime working copies, not a separate long-term in-app vault.
- Any note/body/header mutation that succeeds against the mounted local path must be able to map that local subtree
  back to the original source URI.
- The backend therefore exposes a local-path-to-source sync API so higher layers can push one changed note package or
  file subtree without remounting the whole hub.

## Current Hotspot
- Android editor rollback reports were traced to a one-way mount flow: source URI -> local mounted copy existed, but
  successful `.wsnbody` / `.wsnhead` writes were not being mirrored back into the original SAF source package.
- The platform backend now provides the missing write-back path so mounted note edits can survive beyond the local app
  copy.

## Child Directories
- No child directories.

## Child Files
- `WhatSonAndroidStorageBackend.cpp`
- `WhatSonAndroidStorageBackend.hpp`

## Intended Detailed Sections
- Module responsibilities and architectural layer
- Internal submodule boundaries
- Cross-directory dependencies
- Runtime ownership and lifecycle rules
- Testing strategy and coverage map
- Known hotspots and refactor priorities
