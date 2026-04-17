# `src/app/qml/view/content/editor/ContentsResourceTagController.qml`

## Role
`ContentsResourceTagController.qml` owns RAW `<resource ... />` source concerns for editor-side resource import.

## Responsibilities
- Normalizes imported-entry lists coming from JS arrays or Qt list-like return values.
- Delegates canonical RAW `<resource ... />` text generation to the C++ `ContentsResourceTagTextGenerator` bridge.
- Builds standalone resource-block source text at the current RAW cursor position.
- Counts canonical resource tags and detects accidental tag loss on the legacy inline-editor path.
- Inserts imported resource tags through either `ContentsStructuredDocumentFlow.qml` or
  `ContentsEditorTypingController.qml`, then refreshes body-resource rendering.
