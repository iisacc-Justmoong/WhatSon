# `src/app/qml/view/content/editor/ContentsResourceImportController.qml`

## Role

`ContentsResourceImportController.qml` is the editor-side resource import coordinator shared by the unified
desktop/mobile host.

## Responsibilities

- Delegates drag/drop payload parsing to `ContentsResourceDropPayloadParser.qml`.
- Delegates RAW `<resource ... />` insertion and tag-loss detection to `ContentsResourceTagController.qml`.
- Delegates inline HTML placeholder/image substitution to
  `ContentsInlineResourcePresentationController.qml`.
- Delegates temporary read-only/programmatic-sync guard state to
  `ContentsEditorSurfaceGuardController.qml`.
- Delegates duplicate-import prompt state and conflict policy execution to
  `ContentsResourceImportConflictController.qml`.

## Current Boundary

- The controller no longer mutates `ContentsEditorPresentationProjection` through a RichText surface override.
- RAW resource-tag serialization no longer depends on a host-provided JavaScript string builder.
- Inline resource rendering now stays a pure HTML substitution step owned by the host refresh pipeline.
- The controller keeps the same host-facing import API, but helper-local transient state no longer leaks back into
  arbitrary host properties.
