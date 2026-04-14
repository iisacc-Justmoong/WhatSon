# `src/app/file/import/ResourcesImportViewModel.hpp`

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
- `lastError`
  Stores the last failure message.

## Hooks And Signals

- `canImportUrls(...)`
  Returns whether a file-picker-style URL list can be imported into the current hub. The input may
  be flat or nested (for example picker payloads wrapped inside one `QVariant` entry).
- `importUrls(...)`
  The main file import entrypoint. It still performs the import and same-turn runtime reload as one operation.
- `importUrlsForEditor(...)`
  Imports files and returns per-resource metadata entries (`resourcePath`, `type`, `format`, `bucket`, `assetPath`) so
  the note editor can inject `<resource ... />` links immediately after a drop. This path now skips the automatic
  runtime reload so the editor can finish RAW `.wsnbody` insertion first.
- `importClipboardImage()`, `importClipboardImageForEditor()`
  Clipboard-image variants of the same pipeline. They materialize the clipboard bitmap as a temporary PNG file, then
  reuse the ordinary URL import path so resource package creation, metadata generation, and rollback semantics stay
  identical to drag/drop imports.
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

- Generate a unique `resourceId`.
- Create a flat `resourceId.wsresource` directory.
- Copy the source file name unchanged.
- Write `resource.xml` using `buildMetadataForAssetFile(...)`.
- Append the relative package path to `Resources.wsresources`.
