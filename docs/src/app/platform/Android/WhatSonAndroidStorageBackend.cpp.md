# `src/app/platform/Android/WhatSonAndroidStorageBackend.cpp`

## Status
- Documentation updated for the mounted-hub source-sync implementation.

## Source Metadata
- Source path: `src/app/platform/Android/WhatSonAndroidStorageBackend.cpp`
- Source kind: C++ implementation
- File name: `WhatSonAndroidStorageBackend.cpp`
- Approximate line count: 1408

## Runtime Role
- Implements the Android SAF bridge used by onboarding and startup mount resolution.
- Maintains mount metadata so a local app-data working copy can always be traced back to the original source URI.
- Provides the write-back path that mirrors successful local note mutations back into the real SAF package.

## Main Data Flow
1. `mountHub(...)` validates a SAF directory-backed `.wshub`.
2. The backend allocates a deterministic app-data mount path derived from the source URI hash.
3. `syncSourceDirectoryToLocal(...)` copies the source SAF subtree into that local mount.
4. Metadata is written beside the mount so later code can recover the source URI.
5. After editor persistence succeeds, higher layers call `syncLocalPathToSource(...)`.
6. The backend walks from the changed local file/directory back to the mounted hub root, resolves the relative
   subtree, creates missing SAF child entries when needed, and writes the local bytes back into the original source
   tree.

## Important Helpers
- `findChildEntry(...)`
  Enumerates SAF children and resolves an existing child by name.
- `ensureDirectoryEntry(...)` / `ensureFileEntry(...)`
  Reuse an existing SAF child when present or create one on demand.
- `writeLocalFileToSource(...)`
  Streams one local file into the matching SAF document entry.
- `syncLocalDirectoryToSource(...)`
  Recursively mirrors a local subtree into a SAF directory subtree.
- `mountedHubRootForLocalPath(...)`
  Climbs from an arbitrary local path to the nearest mounted `.wshub` root so callers can pass either a single file or
  a whole note-package directory.

## Failure Semantics
- Local paths outside mounted hubs return success without side effects.
- Missing mount metadata, missing local files, SAF traversal failures, or SAF write failures return a concrete error so
  higher layers can mark persistence as failed instead of pretending the source of truth was updated.
- Child lookup now distinguishes “entry does not exist yet” from “listing children failed”, preventing accidental child
  creation after a SAF enumeration error.

## Architectural Note
- This module keeps Android source synchronization below onboarding/startup policy code but above the generic editor and
  note-model layers. Higher-level persistence code can therefore request mounted-path write-back without learning JNI or
  SAF-specific details.

## Primary Callers
- `src/app/models/onboarding/OnboardingHubController.cpp`
- `src/app/runtime/startup/WhatSonStartupHubResolver.cpp`
- `src/app/models/file/note/ContentsNoteManagementCoordinator.cpp`
