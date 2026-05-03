# `src/app/models/editor/resource/ContentsEditorSurfaceGuardController.hpp`

## Responsibility

Declares the guard state used while resource imports temporarily manipulate the editor presentation surface.

## Current Contract

- Exposes resource-drop and programmatic-surface-sync state to QML.
- Accepts `editorInputPolicyAdapter` as an optional native-input authority for composition and focused-input restore
  decisions.
