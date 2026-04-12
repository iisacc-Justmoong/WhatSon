# `src/app/file/validator/ContentsStructuredTagValidator.cpp`

## Responsibility
Implements the structured-tag validator's opt-in direct-write correction helper.

## Direct Correction Flow
1. Receive a parser/renderer correction suggestion (`sourceText`, `correctedSourceText`, `verification`).
   An opting-in QML host may use the explicit-note entrypoint `requestStructuredCorrectionForNote(...)` so the
   correction target note is fixed at the moment the signal is handled.
2. Resolve the selected note directory from `contentViewModel.noteDirectoryPathForNoteId(...)`.
3. Queue the file read/write correction work onto a worker thread instead of running it inline on the QML signal turn.
4. Persist the canonical corrected RAW source back into `.wsnbody` through `updateNote(...)`.
5. Apply the updated body snapshot back into the bound content view model when possible.
6. Refresh tracked note statistics, then reload note metadata only when the bound content view model
   cannot absorb the persisted body snapshot directly.

## Authority Rule
- This validator is intentionally not read-only when enabled.
- Authority is opt-in and disabled by default so editor hosts can keep note-open and typing on the single
  editor-session persistence path.
- Once a correction suggestion reaches this object and authority is enabled, the validator writes the file directly
  instead of merely reporting lint issues.

## Safety Rules
- The validator refuses to write when:
  - correction authority is disabled
  - `noteId` is empty
  - no note directory path can be resolved
  - the corrected source text is empty or identical to the current source
- Successful identical repeated requests are deduplicated by note/source/corrected-source tuple.
- While one correction is already running, later identical requests are absorbed into the pending queue instead of
  opening another file I/O turn on the UI thread.
- When the active hierarchy view model implements `applyPersistedBodyStateForNote(...)`, a successful
  correction skips the extra immediate `reloadNoteMetadataForNote(...)` round-trip to avoid back-to-back
  note-list refreshes for the same note.

## UI Synchronization
- On success, the validator emits `correctionApplied(...)` so a host that explicitly opted into authority can replace
  the in-editor RAW buffer with the canonical corrected source.
- The emitted success/failure signals represent completion of a worker-thread correction request, not synchronous
  completion inside the parser callback turn.
