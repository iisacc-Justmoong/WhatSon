# `src/app/models/editor/resource/ContentsResourceImportConflictController.cpp`

## Responsibility

Implements duplicate-resource import decision flow.

## Current Behavior

- Stores pending conflict payloads until the user chooses an import policy.
- Emits explicit requests for overwrite, keep-both, or cancellation handling.
- Clears pending state after the decision is consumed.
