# `src/app/platform/Android/WhatSonAndroidStorageBackend.hpp`

## Status
- Documentation updated for the mounted-hub write-back flow.

## Source Metadata
- Source path: `src/app/platform/Android/WhatSonAndroidStorageBackend.hpp`
- Source kind: C++ header
- File name: `WhatSonAndroidStorageBackend.hpp`
- Approximate line count: 70

## Declared Role
- Defines the Android SAF storage façade used by onboarding, startup hub resolution, and mounted-hub persistence.
- Keeps platform-specific document-tree IO behind the `Bridge` interface so JNI-backed production code and test doubles
  can share the same higher-level synchronization logic.

## Public Types
- `EntryMetadata`
  Normalized SAF entry metadata used across stat/list/create/read/write helpers.
- `Bridge`
  Abstract document-tree bridge for stat/list/create/read/write primitives.

## Public API
- `resolveHubSelection(...)`
  Resolves a picker target into one or more mountable `.wshub` document URIs.
- `mountHub(...)`
  Materializes a source URI into a deterministic app-local mounted working copy and persists mount metadata that links
  the local path back to the source URI.
- `exportLocalHubToDirectory(...)`
  Creates a new SAF-backed hub from a local scaffold, then turns that result into the mounted working copy.
- `syncLocalPathToSource(...)`
  Maps a mounted local file or directory back to its original SAF subtree and mirrors the local bytes into the source
  tree. This is the key API that closes the previous one-way mount gap.
- `isMountedHubPath(...)` / `mountedHubSourceUri(...)`
  Recover mount metadata so higher layers can distinguish a mounted Android hub from an ordinary desktop filesystem
  path.

## Invariants
- The Android document URI stays the source of truth.
- The mounted local `.wshub` is a runtime working copy only.
- `syncLocalPathToSource(...)` succeeds as a no-op for non-mounted local paths, so shared higher-level persistence code
  can call it without branching per platform first.

## Main Collaborators
- `src/app/viewmodel/onboarding/OnboardingHubController.*`
- `src/app/runtime/startup/WhatSonStartupHubResolver.*`
- `src/app/models/file/note/ContentsNoteManagementCoordinator.*`
