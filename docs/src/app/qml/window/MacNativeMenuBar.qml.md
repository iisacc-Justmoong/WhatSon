# `src/app/qml/window/MacNativeMenuBar.qml`

## Role

This file defines the macOS global menu bar. It currently exposes two user-facing paths.

- `File > Import File...`
  Opens a native multi-file picker and forwards the selected local files into the current hub resource import flow.
- `Window > Onboarding`
  Reopens the root window onboarding helper.

## Required Properties

- `hostWindow`
  The root window object used by the `Window` menu actions.
- `resourcesImportViewModel`
  The resource import backend that accepts the selected local file URLs.

## Import Flow

1. The user triggers `Import File...`.
2. `FileDialog.OpenFiles` collects one or more local files.
3. On accept, the menu bar calls `resourcesImportViewModel.importUrls(selectedFiles)`.
4. If the import fails, it reads `lastError` and opens a modal failure dialog.

Successful imports rely on the resource runtime reload to refresh the UI immediately, so this menu does not emit a
separate success toast.
