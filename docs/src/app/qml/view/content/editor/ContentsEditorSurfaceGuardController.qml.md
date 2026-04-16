# `src/app/qml/view/content/editor/ContentsEditorSurfaceGuardController.qml`

## Role
`ContentsEditorSurfaceGuardController.qml` owns temporary editor-surface guard state for resource import turns.

## Responsibilities
- Tracks one resource-drop guard token and read-only guard-active state.
- Tracks programmatic editor-surface sync depth independently from the desktop/mobile hosts.
- Restores the live editor surface from either logical text or rendered RichText after import-side rewrites.
