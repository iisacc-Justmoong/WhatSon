# `src/app/file/import/ResourcesImportViewModel.cpp`

## Responsibility

The implementation now supports two closely related sequences.

1. Common import path:
   Filter the input URL list, copy each file into a `.wsresource` package, and rewrite `Resources.wsresources`.
2. Default import path (`importUrls(...)` / compatibility callers):
   Invoke the resources runtime reload callback immediately after the store rewrite succeeds.
3. Editor drop path (`importUrlsForEditor(...)`):
   Return normalized imported-entry metadata first, let the editor insert canonical `<resource ... />` RAW source into
   `.wsnbody`, and only then call `reloadImportedResources()` from QML.

## Import Semantics

- The source file name is preserved as the package asset file name.
- The package id is derived from the file base name and receives a suffix when it would collide with an existing
  package.
- `type`, `bucket`, and `format` are assigned by the rules in `WhatSonResourcePackageSupport.hpp`.
- If multiple resource roots exist (`.wsresources` and `*.wsresources`), import prefers the root already referenced
  by existing `Resources.wsresources` entries so new packages stay in the same lineage.
- `importUrlsForEditor(...)` reuses the same import pipeline as `importUrls(...)` but also returns normalized metadata
  maps so the editor can insert canonical self-closing `<resource ... />` tags without reparsing
  `Resources.wsresources`.
- That editor-return path intentionally defers the runtime reload callback until the view finishes RAW note mutation.
  This avoids editor rebind/reconcile churn between `.wsresource` package creation and the `.wsnbody` insertion turn
  that links those packages into the note.
- URL extraction accepts both flat URL arrays and nested picker payload variants (for example a
  `QVariantList` that wraps a file-dialog URL list), then flattens them into unique local files.
- The editor drag/drop path also consumes native `drop.urls` payloads plus `text/uri-list` fallback lines, so Finder,
  Explorer, and other host file managers stay on the same rollback-safe import pipeline as menu/file-picker imports.
- QML callers should still treat the `QVariantList` return from `importUrlsForEditor(...)` as a Qt list-like value,
  not only as a strict JS `Array`, because post-import body insertion may otherwise skip valid imported entries.

## Failure Rule

If package creation or `Resources.wsresources` rewriting fails, every package directory created during that turn is
rolled back. If persistence succeeds but the runtime refresh callback fails, the ViewModel now removes the imported
packages and restores the previous `Resources.wsresources` contents before emitting the failure signal.

## Current Callers

- The macOS global menu bar `File > Import File...` action.
- Desktop/mobile editor drop surfaces, which import the files and then inject `<resource ...>` source tags into the
  current note before requesting the deferred resources runtime reload.
- The compatibility wrappers that keep older drag/drop-style callers on the same code path.
