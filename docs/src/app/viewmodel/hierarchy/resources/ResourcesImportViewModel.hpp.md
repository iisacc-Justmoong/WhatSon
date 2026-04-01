# `src/app/viewmodel/hierarchy/resources/ResourcesImportViewModel.hpp`

## Role

This ViewModel translates external local file URLs into resource package imports.

- It stores the currently active `.wshub` path.
- It decides whether incoming file URLs are importable.
- It executes the import and then triggers the runtime reload callback.

## Public Contract

- `currentHubPath`
  The current import target hub. Import is rejected while this path is empty.
- `busy`
  Indicates that import and the follow-up reload are in progress.
- `lastError`
  Stores the last failure message.

## Hooks And Signals

- `canImportUrls(...)`
  Returns whether a file-picker-style URL list can be imported into the current hub. The input may
  be flat or nested (for example picker payloads wrapped inside one `QVariant` entry).
- `importUrls(...)`
  The main file import entrypoint.
- `importUrlsForEditor(...)`
  Imports files and returns per-resource metadata entries (`resourcePath`, `type`, `format`, `bucket`, `assetPath`) so
  the note editor can inject `<resource ...>` links immediately after a drop.
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
