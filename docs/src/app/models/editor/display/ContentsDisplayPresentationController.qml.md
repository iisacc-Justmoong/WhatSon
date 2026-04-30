# `src/app/models/editor/display/ContentsDisplayPresentationController.qml`

## Responsibility

Owns QML-runtime presentation refresh orchestration for the editor display host.

## Boundary

- Coordinates projection refresh, inline resource presentation, and trace dispatch.
- Delegates public access through `ContentsDisplayPresentationController`.
- Does not mutate RAW note source.
