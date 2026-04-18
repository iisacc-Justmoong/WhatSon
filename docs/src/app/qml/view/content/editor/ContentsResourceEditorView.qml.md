# `src/app/qml/view/content/editor/ContentsResourceEditorView.qml`

## Responsibility

`ContentsResourceEditorView.qml` is the dedicated center-surface resource editor used when the active hierarchy is
resource-backed instead of note-backed.

## Rendering Contract

- Receives one `resourceEntry` payload and renders:
  - a metadata/header strip
  - a large preview panel
  - an empty/unsupported fallback state
- The first supported dedicated resource-editor format is bitmap image preview.
- Image rendering is delegated to `ContentsResourceViewer.qml` and guarded by `ResourceBitmapViewer` state, so the
  editor only mounts an actual preview when the selected resource resolves to a compatible bitmap target.

## Inputs

- `displayColor`
  Inherits the current content-surface background color from `ContentViewLayout.qml`.
- `resourceEntry`
  Direct selected-resource payload forwarded from the active resource list model.

## Interaction

- Exposes `viewHookRequested` so outer layout shells can keep the same generic hook surface used by the note editor
  and calendar pages.
- Keeps unsupported resources visible as metadata plus a clear fallback message instead of silently dropping the
  center panel.

## Tests

- Automated regression coverage for the routing contract lives in
  `test/cpp/whatson_cpp_regression_tests.cpp` through the surface-mode helper and `ResourceBitmapViewer`.
- Regression checklist:
  - entering the Resources hierarchy with an image selection must replace the note editor with this resource editor
  - bitmap-compatible resources must render through `ContentsResourceViewer.qml`
  - unsupported resource formats must keep the metadata card visible and show the unsupported-state message
