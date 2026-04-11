# `src/app/file/validator/ContentsStructuredTagValidator.cpp`

## Responsibility
Implements the structured-tag validator's direct-write correction authority.

## Direct Correction Flow
1. Receive a parser/renderer correction suggestion (`sourceText`, `correctedSourceText`, `verification`).
   QML hosts use the explicit-note entrypoint `requestStructuredCorrectionForNote(...)` so the correction target note
   is fixed at the moment the signal is handled.
2. Resolve the selected note directory from `contentViewModel.noteDirectoryPathForNoteId(...)`.
3. Read the materialized note package through `WhatSonLocalNoteFileStore`.
4. Persist the canonical corrected RAW source back into `.wsnbody` immediately through `updateNote(...)`.
5. Apply the updated body snapshot back into the bound content view model when possible.
6. Refresh tracked note statistics and reload note metadata so the UI/runtime state converges on the corrected file.

## Authority Rule
- This validator is intentionally not read-only.
- Once a correction suggestion reaches this object and authority is enabled, the validator writes the file directly
  instead of merely reporting lint issues.
- The validator therefore acts as the file-layer owner of structured-tag correction, not as a passive warning sink.

## Safety Rules
- The validator refuses to write when:
  - correction authority is disabled
  - `noteId` is empty
  - no note directory path can be resolved
  - the corrected source text is empty or identical to the current source
- Successful identical repeated requests are deduplicated by note/source/corrected-source tuple.

## UI Synchronization
- On success, the validator emits `correctionApplied(...)` so QML can immediately replace the in-editor RAW buffer with
  the canonical corrected source and avoid re-saving stale malformed text back over the fixed file.
