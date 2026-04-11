# `src/app/file/validator/ContentsStructuredTagValidator.hpp`

## Role
`ContentsStructuredTagValidator` is the QML-facing structured-tag correction authority.

## Public Contract
- `contentViewModel`
  - Provides the resolver hooks needed to find the current note directory and refresh the note snapshot after a direct correction write.
- `noteId`
  - Identifies which note the validator is allowed to rewrite.
- `correctionAuthorityEnabled`
  - Gates whether parser-side correction suggestions may trigger direct file writes.
- `lastCorrectionVerification`
  - Exposes the last parser/renderer verification payload that led to a correction attempt.
- `lastCorrectedSourceText`
  - Exposes the last canonical RAW source text the validator attempted to persist.
- `lastCorrectionError`
  - Exposes the last direct-correction failure message.
- `requestStructuredCorrection(sourceText, correctedSourceText, verification)`
  - Slot/QML-callable entry that accepts a parser-side correction suggestion for the currently bound `noteId`.
- `requestStructuredCorrectionForNote(noteId, sourceText, correctedSourceText, verification)`
  - Explicit-note variant used by QML hosts so note switches cannot race correction requests onto the wrong file.

## Signals
- `correctionApplied(noteId, correctedSourceText, verification)`
- `correctionFailed(noteId, sourceText, errorMessage, verification)`

## Registration Constraint
- This QObject type is registered via `qmlRegisterType<ContentsStructuredTagValidator>()`.
- The class must remain non-`final` so Qt can wrap it through `QQmlElement<T>`.
