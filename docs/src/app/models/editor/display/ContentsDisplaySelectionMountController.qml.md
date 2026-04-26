# `src/app/models/editor/display/ContentsDisplaySelectionMountController.qml`

## Responsibility

Owns QML-runtime selection and note-body mount orchestration for the editor display host.

## Boundary

- Coordinates selected-note snapshot polling, reconcile, focus scheduling, and mount delivery.
- Delegates public access through `ContentsDisplaySelectionMountViewModel`.
- Sends focus restoration through `ContentsActiveEditorSurfaceAdapter` so the controller depends on the active editor
  surface contract instead of directly targeting `ContentsInlineFormatEditor.qml` or `ContentsStructuredDocumentFlow.qml`.
- Does not own visual layout or document rendering.
