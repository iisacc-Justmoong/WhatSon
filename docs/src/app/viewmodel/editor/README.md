# `src/app/viewmodel/editor`

## Responsibility

Owns editor-facing C++ ViewModels that publish narrow QML-callable command surfaces between declarative editor views
and editor-domain models under `src/app/models/editor`.

## Boundary

- Visual block and layout surfaces stay under `src/app/qml/view/content/editor`.
- RAW source parsing, rendering, and low-level calculation models stay under `src/app/models/editor`.
- QML files are prohibited in this tree. Timing, pointer surfaces, and JS orchestration that still need QML primitives
  belong under `src/app/models/editor`.
- C++ ViewModels may delegate to model-side controllers, but each class must remain single-purpose and must not become
  persisted document authority.

## Current Modules

- `display`
  Publishes selection/mount, presentation, mutation, and geometry ViewModels for the unified note editor host.
