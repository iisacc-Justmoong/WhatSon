# `src/app/models/editor/display/ContentsDisplaySelectionMountController.qml`

## Responsibility

Owns QML-runtime selection and note-body mount orchestration for the editor display host.

## Boundary

- Coordinates selected-note snapshot polling, reconcile, focus scheduling, and mount delivery.
- Delegates public access through `ContentsDisplaySelectionMountViewModel`.
- Does not own visual layout or document rendering.
