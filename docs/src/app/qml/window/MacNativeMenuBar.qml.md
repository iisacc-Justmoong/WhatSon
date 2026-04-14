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
3. On accept, the menu bar normalizes picker output through `selectedImportUrls()`, then asks
   `resourcesImportViewModel.inspectImportConflictForUrls(selectedFiles)` whether any incoming asset name already
   exists in the current `*.wsresources` store.
4. If a duplicate exists, the menu bar opens an `LV.Alert` mounted on the host window content surface and lets the
   user choose `Overwrite`, `Keep Both`, or `Cancel Import`.
5. After the user chooses a policy, the menu bar calls
   `resourcesImportViewModel.importUrlsWithConflictPolicy(selectedFiles, policy)`.
6. If the import fails, it reads `lastError` and opens a modal failure dialog.

`selectedImportUrls()` intentionally merges both `selectedFiles` and `selectedFile` paths so import
works across picker payload shape differences on macOS native dialog backends.

Successful imports rely on the resource runtime reload to refresh the UI immediately, so this menu does not emit a
separate success toast.
