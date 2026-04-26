# `src/app/models/editor/resource`

## Responsibility
Owns editor-side resource import, RAW resource-tag insertion, and inline resource presentation coordination.

## Current Modules
- `ContentsResourceImportController.qml`
  Public import coordinator mounted by the editor host.
- `ContentsResourceDropPayloadParser.qml`
  Normalizes drag/drop payloads into importable resource URLs.
- `ContentsResourceImportConflictController.qml`
  Owns duplicate-resource prompt state.
- `ContentsResourceTagController.qml`
  Inserts canonical `<resource ... />` tags into RAW source.
- `ContentsInlineResourcePresentationController.qml`
  Builds editor-surface inline resource HTML placeholders.
- `ContentsEditorSurfaceGuardController.qml`
  Guards programmatic editor-surface sync during resource import turns.

## Boundary
- Visual resource cards and viewers remain under `src/app/qml/view/content/editor`.
- File/resource storage and bitmap/PDF rendering backends remain under `src/app/models/file`.
