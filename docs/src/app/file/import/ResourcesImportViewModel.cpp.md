# `src/app/file/import/ResourcesImportViewModel.cpp`

## Responsibility

The implementation now supports two closely related sequences.

1. Common import path:
   Filter the input URL list, copy each file into a `.wsresource` package, and rewrite `Resources.wsresources`.
2. Default import path (`importUrls(...)` / compatibility callers):
   Invoke the resources runtime reload callback immediately after the store rewrite succeeds, but reject same-name
   conflicts unless the caller has already chosen an explicit overwrite/keep-both policy.
3. Editor drop path (`importUrlsForEditor(...)`):
   Return normalized imported-entry metadata first, let the editor insert canonical `<resource ... />` RAW source into
   `.wsnbody`, and only then call `reloadImportedResources()` from QML.
4. Clipboard-image path (`importClipboardImage(...)` / `importClipboardImageForEditor(...)`):
   Read the current clipboard image, serialize it as a temporary `clipboard-image.png`, and then hand that local file
   back into the same import pipeline used by drag/drop.
5. Conflict-inspection path (`inspectImportConflictForUrls(...)` / `inspectClipboardImageImportConflict()`):
   Resolve the current hub/resources root, compare the incoming source file name against existing package asset names,
   and return the first duplicate so QML can ask the user what to do before the import mutates storage.

## Import Semantics

- The source file name is preserved as the package asset file name.
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
- Clipboard-image import intentionally does not create a second bespoke packaging path.
  Temporary PNG materialization means package-id generation, `resource.xml` creation, rollback, and later runtime
  reload all stay on the same battle-tested import path as ordinary file drops.
- That editor-return path intentionally defers the runtime reload callback until the view finishes RAW note mutation.
  This avoids editor rebind/reconcile churn between `.wsresource` package creation and the `.wsnbody` insertion turn
  that links those packages into the note.
- URL extraction accepts both flat URL arrays and nested picker payload variants (for example a
  `QVariantList` that wraps a file-dialog URL list), then flattens them into unique local files.
- The editor drag/drop path also consumes native `drop.urls` payloads plus `text/uri-list` fallback lines, so Finder,
  Explorer, and other host file managers stay on the same rollback-safe import pipeline as menu/file-picker imports.
- Clipboard availability is tracked as a live property by listening to `QClipboard::dataChanged()`, so the editor can
  enable image-paste interception only while the clipboard still contains image data.
- Clipboard image extraction now also accepts browser/platform variants that do not arrive as a plain `image/*` MIME:
  image-like platform MIME names such as `public.png` / `public.tiff`, plus `text/html` or plain-text `data:image/...`
  payloads now feed the same temporary-PNG import path.
  Copying an image from a browser page therefore still resolves as a binary image import even when the clipboard uses
  HTML/data-URL transport instead of `hasImage()`.
- Clipboard conflict inspection intentionally uses the canonical temporary asset name `clipboard-image.png`, so repeated
  pasted-image imports now hit the same duplicate-policy alert instead of silently creating numbered packages.
- QML callers should still treat the `QVariantList` return from `importUrlsForEditor(...)` as a Qt list-like value,
  not only as a strict JS `Array`, because post-import body insertion may otherwise skip valid imported entries.

## Failure Rule

If package creation or `Resources.wsresources` rewriting fails, every package directory created during that turn is
rolled back.
If overwrite mode was active, the ViewModel also restores the original staged package directory before emitting the
failure signal.
If persistence succeeds but the runtime refresh callback fails, the ViewModel now removes newly created packages,
restores overwritten packages, and restores the previous `Resources.wsresources` contents before emitting the failure
signal.

## Current Callers

- The macOS global menu bar `File > Import File...` action.
- Desktop/mobile editor drop surfaces, which import the files and then inject `<resource ...>` source tags into the
  current note before requesting the deferred resources runtime reload.
- The compatibility wrappers that keep older drag/drop-style callers on the same code path.
