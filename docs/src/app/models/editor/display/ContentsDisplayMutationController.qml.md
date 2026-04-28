# `src/app/models/editor/display/ContentsDisplayMutationController.qml`

## Responsibility

Owns QML-runtime mutation orchestration for the editor display host.

## Boundary

- Applies incoming RAW `.wsnbody` source text directly to the display host before persistence and parser projection.
- Delegates public access through `ContentsDisplayMutationViewModel`.
- Must preserve RAW source as the only write authority.
- Does not route end-of-document focus or block-focus restoration through a generic editor-surface adapter.
  Those requests go directly to the mounted `ContentsStructuredDocumentFlow.qml` host.
