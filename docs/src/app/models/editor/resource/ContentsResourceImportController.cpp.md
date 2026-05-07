# `src/app/models/editor/resource/ContentsResourceImportController.cpp`

## Responsibility

Implements the editor resource import coordinator.

## Current Contract

- Keeps resource-import child controllers synchronized from the host-facing properties before each public operation.
- No longer forwards display-only resource frame properties to an inline presentation child; resource import remains a
  RAW tag insertion and guard/restore coordinator.
- Forwards `editorInputPolicyAdapter` into `ContentsEditorSurfaceGuardController`, allowing pending resource-import
  presentation restores to defer while native composition or focused native text input policy is active.
- Keeps persistence out of the resource import layer; RAW resource-tag writes still route through the editor session.
