# `src/app/models/editor/tags/ContentsStructuredTagValidator.cpp`

## Responsibility
Implements the structured-tag validator's advisory correction gate.

## Advisory Flow
1. Receive a parser/renderer correction suggestion (`sourceText`, `correctedSourceText`, `verification`).
   A QML host may use the explicit-note entrypoint `requestStructuredCorrectionForNote(...)` so the
   correction target note is fixed at the moment the signal is handled.
2. Normalize the reported source/correction payloads for comparison.
3. Reject the request immediately with a structured failure signal instead of performing file I/O.
4. Surface the correction payload as advisory state only (`lastCorrectionVerification`, `lastCorrectedSourceText`,
   `lastCorrectionError`).

## Authority Rule
- Automatic RAW correction is now disabled regardless of the `correctionAuthorityEnabled` flag.
- This keeps `.wsnbody` RAW authority on the editor mutation path only; parser-side validation may suggest a
  correction, but it must not rewrite note source behind the editor's back.

## Safety Rules
- The validator refuses to apply any correction when:
  - `noteId` is empty
  - the corrected source text is empty
  - the corrected source text is identical to the current source
- For a real correction delta, it reports a failure explaining that automatic RAW rewrites are disabled.

## UI Synchronization
- The validator emits `correctionFailed(...)` for rejected automatic rewrites so a host can surface that state without
  mutating the file.
- `correctionApplied(...)` remains part of the API surface for compatibility, but this implementation no longer emits
  it as part of passive parser-driven correction attempts.
