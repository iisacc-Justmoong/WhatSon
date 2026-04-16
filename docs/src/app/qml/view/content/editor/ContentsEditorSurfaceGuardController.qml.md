# `src/app/qml/view/content/editor/ContentsEditorSurfaceGuardController.qml`

## Role

`ContentsEditorSurfaceGuardController.qml` owns temporary editor-surface guard state for resource import turns.

## Responsibilities

- Tracks one resource-drop guard token and read-only guard-active state.
- Tracks programmatic editor-surface sync depth independently from the desktop/mobile hosts.
- Restores the live editor surface from the projection's logical plain text after import-side rewrites.

## Current Contract

- The controller no longer chooses between logical text and a separate RichText editor surface.
- Restoration always targets the plain-text input buffer exposed by `ContentsInlineFormatEditor.qml`.
- The guard remains necessary so drag/drop resource import cannot be misread as authored editor typing.
