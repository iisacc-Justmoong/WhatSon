# `src/app/models/editor/display/ContentsDisplaySelectionMountController.qml`

## Responsibility

Owns QML-runtime selection and note-body mount orchestration for the editor display host.

## Boundary

- Coordinates selected-note snapshot polling, reconcile, focus scheduling, and mount delivery.
- Delegates public access through `ContentsDisplaySelectionMountViewModel`.
- Sends focus restoration directly to `ContentsStructuredDocumentFlow.qml`, which is the canonical note document host
  once a session is bound.
- Does not own visual layout or document rendering.
