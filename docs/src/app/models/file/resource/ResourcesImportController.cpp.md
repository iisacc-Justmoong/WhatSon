# `src/app/models/file/resource/ResourcesImportController.cpp`

## Responsibility

The implementation supports these related sequences.

1. Common import path:
   Filter the input URL list, copy each file into a `.wsresource` package, and rewrite `Resources.wsresources`.
2. Default import path (`importUrls(...)` / compatibility callers):
   Invoke the resources runtime reload callback immediately after the store rewrite succeeds, but reject same-name
   conflicts unless the caller has already chosen an explicit overwrite/keep-both policy.
3. Editor drop path (`importUrlsForEditor(...)`):
   Return normalized imported-entry metadata first, let the editor insert canonical `<resource ... />` RAW source into
   `.wsnbody`, and only then call `reloadImportedResources()` from QML.
4. Conflict-inspection path (`inspectImportConflictForUrls(...)`):
   Resolve the current hub/resources root, compare the incoming source file name against existing package asset names,
   and return the first duplicate so QML can ask the user what to do before the import mutates storage.

## Import Semantics

- The source file name is preserved as the package asset file name.
- Every newly created or overwritten `.wsresource` package now also writes `annotation.png` beside the original asset
  and `resource.xml`.
  - bitmap image imports get a transparent annotation bitmap that matches the source image size
  - non-image imports currently receive a minimal transparent bitmap placeholder until dedicated non-image annotation
    sizing rules arrive
- Same-name conflicts are detected by the incoming asset file name, not by package id.
- If the caller selects `KeepBoth`, the package id is still derived from the file base name and receives a suffix when
  it would collide with an existing package.
- If the caller selects `Overwrite`, the import path temporarily stages the existing package aside, recreates the
  package at the original path, writes new metadata with the original `resourceId`/`resourcePath`, and keeps that
  original package address stable for existing `<resource ... />` links.
- `type`, `bucket`, and `format` are assigned by the rules in `WhatSonResourcePackageSupport.hpp`.
- If multiple resource roots exist (`.wsresources` and `*.wsresources`), import prefers the root already referenced
  by existing `Resources.wsresources` entries so new packages stay in the same lineage.
- `importUrlsForEditor(...)` reuses the same import pipeline as `importUrls(...)` but also returns normalized metadata
  maps so the editor can insert canonical self-closing `<resource ... />` tags without reparsing
  `Resources.wsresources`.
- Editor-facing import methods return those metadata maps only after the `.wsresources/<id>.wsresource` package and
  `Resources.wsresources` list entry have both been written; note-body insertion must link that package path rather
  than embed incoming asset payloads directly.
- The same shared import path now also owns annotation-canvas generation, so overwrite/rollback and fresh package
  creation cannot diverge on whether `annotation.png` exists.
- That editor-return path intentionally defers the runtime reload callback until the view finishes RAW note mutation.
  This avoids editor rebind/reconcile churn between `.wsresource` package creation and the `.wsnbody` insertion turn
  that links those packages into the note.
- URL extraction accepts both flat URL arrays and nested picker payload variants (for example a
  `QVariantList` that wraps a file-dialog URL list), then flattens them into unique local files.
- The editor drag/drop path also consumes native `drop.urls` payloads plus `text/uri-list` fallback lines, so Finder,
  Explorer, and other host file managers stay on the same rollback-safe import pipeline as menu/file-picker imports.
- QML callers should still treat the `QVariantList` return from `importUrlsForEditor(...)` as a Qt list-like value,
  not only as a strict JS `Array`, because post-import body insertion may otherwise skip valid imported entries.
- The former clipboard-image paste path has been removed from this Controller. New image-paste work must define a new
  C++ clipboard import contract instead of depending on the deleted paste invokables.

## Failure Rule

If package creation or `Resources.wsresources` rewriting fails, every package directory created during that turn is
rolled back.
If overwrite mode was active, the Controller also restores the original staged package directory before emitting the
failure signal.
If persistence succeeds but the runtime refresh callback fails, the Controller now removes newly created packages,
restores overwritten packages, and restores the previous `Resources.wsresources` contents before emitting the failure
signal.

## Current Callers

- The macOS global menu bar `File > Import File...` action.
- Desktop/mobile editor drop surfaces, which import the files and then inject `<resource ...>` source tags into the
  current note before requesting the deferred resources runtime reload.
- The compatibility wrappers that keep older drag/drop-style callers on the same code path.
