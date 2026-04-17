# `src/app/qml/view/content/editor/ContentsResourceTagController.qml`

## Role
`ContentsResourceTagController.qml` owns RAW `<resource ... />` source concerns for editor-side resource import.

## Responsibilities
- Normalizes imported-entry lists coming from JS arrays or Qt list-like return values.
- Delegates canonical RAW `<resource ... />` text generation to the C++ `ContentsResourceTagTextGenerator` bridge.
- Resolves the current RAW insertion offset from the live structured-block cursor when available, then falls back to
  the legacy editor cursor mapping.
- Delegates the final RAW splice payload to `ContentsStructuredDocumentMutationPolicy` and applies that payload through
  the host `applyDocumentSourceMutation(...)` path.
- Counts canonical resource tags and detects accidental tag loss on the legacy inline-editor path.
- Falls back to `ContentsEditorTypingController.qml` only when the host mutation handler is unavailable, then refreshes
  body-resource rendering.

## Ownership Note
- Because the root object is `QtObject`, helper bridges must stay attached through explicit properties rather than
  anonymous child-object declarations.
