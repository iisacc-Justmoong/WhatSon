# `src/app/models/file/import/ResourcesImportViewModel.hpp`

## Role

This ViewModel translates external local file URLs into resource package imports.

- It stores the currently active `.wshub` path.
- It decides whether incoming file URLs are importable.
- It executes the import and can trigger the runtime reload callback either immediately or after an editor-owned
  follow-up step.

## Public Contract

- `currentHubPath`
  The current import target hub. Import is rejected while this path is empty.
- `busy`
  Indicates that import is in progress. Deferred editor-owned runtime reloads run after import completes.
- `clipboardImageAvailable`
  Reflects whether the system clipboard currently exposes an importable image payload. Editor paste shortcuts can bind
  to this flag so image paste is intercepted only when the clipboard actually contains image data.
- `refreshClipboardImageAvailabilitySnapshot()`
  Forces one immediate clipboard rescan and returns the current image-availability decision. Paste handlers use this
  when the app regains focus after the user copied an image in another process and QML needs a fresh answer on the
  shortcut turn itself.
- `lastError`
  Stores the last failure message.

## Hooks And Signals

- `canImportUrls(...)`
  Returns whether a file-picker-style URL list can be imported into the current hub. The input may
  be flat or nested (for example picker payloads wrapped inside one `QVariant` entry).
- `inspectImportConflictForUrls(...)`, `inspectClipboardImageImportConflict()`
  Return the first same-name import conflict against the current `*.wsresources` store as a small map
  (`conflict`, `sourceFileName`, `existingResourcePath`, `existingResourceId`, ...).
  Visual callers use that payload to open an `LV.Alert` before they decide whether the import should overwrite the
  existing package, keep both copies, or stop.
- `importUrls(...)`
  The main file import entrypoint. It now aborts same-name conflicts unless a caller has already chosen a conflict
  policy explicitly.
- `importUrlsWithConflictPolicy(...)`, `importUrlsForEditorWithConflictPolicy(...)`
  Policy-aware variants of the file import path. Callers pass `Abort`, `Overwrite`, or `KeepBoth` after an explicit UI
  decision.
- `importUrlsForEditor(...)`
  Imports files and returns per-resource metadata entries (`resourcePath`, `type`, `format`, `bucket`, `assetPath`) so
  the note editor can inject `<resource ... />` links immediately after a drop.
  This compatibility entrypoint now also aborts same-name conflicts unless the caller switches to the policy-aware
  variant after user confirmation.
- `importClipboardImage()`, `importClipboardImageForEditor()`
  Clipboard-image variants of the same pipeline. They materialize the clipboard bitmap as a temporary PNG file, then
  reuse the ordinary URL import path so resource package creation, metadata generation, and rollback semantics stay
  identical to drag/drop imports.
  The temporary asset file name is now a random 32-character mixed-case alphanumeric key, so clipboard-originated
  resources do not reuse an ambiguous placeholder name inside the resource store.
- `importClipboardImageWithConflictPolicy(...)`, `importClipboardImageForEditorWithConflictPolicy(...)`
  Clipboard-image variants of the same explicit conflict-policy flow.
- `reloadImportedResources()`
  Runs the deferred resources runtime reload for editor-drop callers after they finish `<resource ... />` insertion and
  same-note persistence.
- `canImportDroppedUrls(...)`, `importDroppedUrls(...)`
  Compatibility wrappers that forward legacy drag/drop callers into the same import path.
- `importCompleted(int)`
  Reports the number of imported files after success.
- `operationFailed(QString)`
  Reports persistence or runtime refresh failure.

## Storage Policy

Each input file is written into the current hub `*.wsresources` store with these rules.

- If no same-name conflict exists, generate a unique `resourceId`.
- If a same-name conflict exists and the caller selects `Overwrite`, reuse the existing package path/resource id and
  replace that package contents in-place.
- If a same-name conflict exists and the caller selects `KeepBoth`, fall back to the numbered `resourceId` path.
- Create a flat `resourceId.wsresource` directory.
- Copy the source file name unchanged for ordinary local-file imports.
- Clipboard-image imports first synthesize a random 32-character mixed-case alphanumeric `.png` file name, then copy
  that generated name into the package.
- Generate `annotation.png` as an empty bitmap overlay canvas inside the package.
  - bitmap image imports mirror the source pixel size with a transparent PNG
  - non-bitmap imports fall back to a minimal transparent bitmap placeholder
- Write `resource.xml` using `buildMetadataForAssetFile(...)`, including the package-local annotation path.
- Append the relative package path to `Resources.wsresources`.
