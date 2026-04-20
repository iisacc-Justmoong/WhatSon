# `src/app/qml/view/content/editor/ContentsResourceEditorView.qml`

## Responsibility

`ContentsResourceEditorView.qml` is the dedicated center-surface resource editor used when the active hierarchy is
resource-backed instead of note-backed.

## Rendering Contract

- Receives one `resourceEntry` payload and renders only the resource viewport itself.
- The surface stays visually transparent:
  - no background card
  - no top metadata/header strip
  - no bottom preview/explanation copy
  - no empty/unsupported explanatory label overlay
- The first supported dedicated resource-editor format is bitmap image preview.
- Image rendering is delegated to `ContentsResourceViewer.qml` and guarded by `ResourceBitmapViewer` state, so the
  editor only mounts an actual preview when the selected resource resolves to a compatible bitmap target.
  Until additional formats are promoted into the dedicated editor contract, unsupported or missing selections remain a
  transparent empty surface instead of falling back to metadata chrome.

## Inputs

- `displayColor`
  Kept only as a compatibility input so outer shells do not need a parallel property fork; the dedicated resource
  editor itself does not paint that color and remains transparent.
- `resourceEntry`
  Direct selected-resource payload forwarded from the active resource list model.

## Interaction

- Exposes `viewHookRequested` so outer layout shells can keep the same generic hook surface used by the note editor
  and calendar pages.
- Does not add resource-editor-local labels or metadata scaffolding around the viewer; if nothing renderable is
  available, the center surface stays empty.

## Tests

- Automated regression coverage for the routing contract lives in
  `test/cpp/suites/*.cpp` through the surface-mode helper and `ResourceBitmapViewer`.
- Regression checklist:
  - entering the Resources hierarchy with an image selection must replace the note editor with this resource editor
  - bitmap-compatible resources must render through `ContentsResourceViewer.qml`
  - the dedicated resource editor must not paint its own background or top/bottom metadata copy
  - unsupported resource formats must not reintroduce a metadata card or explanatory overlay text
