# `src/app/models/editor/resource/ContentsResourceImportController.qml`

## Role

`ContentsResourceImportController.qml` is the editor-side resource import coordinator shared by the unified
desktop/mobile host.

## Responsibilities

- Delegates drag/drop payload parsing to `ContentsResourceDropPayloadParser.qml`.
- Delegates RAW `<resource ... />` insertion and tag-loss detection to
  `src/app/models/editor/tags/ContentsResourceTagController.qml`.
- Delegates inline HTML placeholder/image substitution to
  `ContentsInlineResourcePresentationController.qml`.
- Delegates temporary read-only/programmatic-sync guard state to
  `ContentsEditorSurfaceGuardController.qml`.
  Pending surface restores are resumed only after native input composition settles.
- Delegates duplicate-import prompt state and conflict policy execution to
  `ContentsResourceImportConflictController.qml`.
- Exposes `pasteClipboardImageAsResource()` for the live editor `TextEdit` tag-management hook and the command surface
  shortcut path. Both routes share the same conflict prompt, resource import, RAW `<resource ... />` insertion, and
  runtime resource reload flow.

## Current Boundary

- The controller no longer mutates `ContentsEditorPresentationProjection` through a RichText surface override.
- RAW resource-tag serialization no longer depends on a host-provided JavaScript string builder.
- RAW resource-tag insertion now also routes through the host document-source mutation handler instead of depending on
  a structured-flow-local success path.
- Inline resource rendering now stays a pure HTML substitution step owned by the host refresh pipeline.
- The controller keeps the same host-facing import API, but helper-local transient state no longer leaks back into
  arbitrary host properties.
