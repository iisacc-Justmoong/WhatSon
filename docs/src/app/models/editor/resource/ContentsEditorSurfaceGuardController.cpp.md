# `src/app/models/editor/resource/ContentsEditorSurfaceGuardController.cpp`

## Responsibility

Implements resource-import presentation surface guarding.

## Current Contract

- Defers presentation restore while native composition is active.
- Consults `editorInputPolicyAdapter` before falling back to the mounted editor control's local helper functions.
- Marks programmatic sync depth for two event-loop turns so editor sessions can distinguish host-driven text updates
  from user edits.
