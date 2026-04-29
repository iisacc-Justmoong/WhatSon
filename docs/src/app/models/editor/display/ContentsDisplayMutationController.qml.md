# `src/app/models/editor/display/ContentsDisplayMutationController.qml`

## Responsibility

Owns QML-runtime mutation orchestration for the editor display host.

## Boundary

- Applies incoming RAW `.wsnbody` source text through `ContentsEditorSessionController::commitRawEditorTextMutation(...)`
  before parser projection observes the updated session source.
- Delegates public access through `ContentsDisplayMutationViewModel`.
- Must preserve RAW source as the only write authority.
- Must not write `contentsView.editorText`, mark local editor authority, or schedule persistence directly.
- Does not route trailing-margin focus or block-focus restoration through a generic editor-surface adapter.
  Those requests go directly from the mounted surface host to `ContentsStructuredDocumentFlow.qml`.
