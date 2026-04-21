# `src/app/models/file/validator/ContentsStructuredTagValidator.hpp`

## Role
`ContentsStructuredTagValidator` is a QML-facing structured-tag correction advisory helper.

## Public Contract
- `contentViewModel`
  - Preserved for compatibility with existing QML wiring, even though automatic direct correction writes are disabled.
- `noteId`
  - Identifies which note the validator is reporting against.
- `correctionAuthorityEnabled`
  - Preserved as a compatibility flag only.
  - Automatic parser-side file rewrites remain disabled so note RAW changes stay on the editor mutation path.
- `lastCorrectionVerification`
  - Exposes the last parser/renderer verification payload that led to a rejected automatic correction attempt.
- `lastCorrectedSourceText`
  - Exposes the last canonical RAW source text that was suggested but not auto-persisted.
- `lastCorrectionError`
  - Exposes the last advisory rejection message.
- `requestStructuredCorrection(sourceText, correctedSourceText, verification)`
  - Slot/QML-callable entry that accepts a parser-side correction suggestion for the currently bound `noteId` and rejects automatic RAW mutation.
- `requestStructuredCorrectionForNote(noteId, sourceText, correctedSourceText, verification)`
  - Explicit-note variant used by QML hosts so note switches cannot race advisory correction requests onto the wrong file.

## Signals
- `correctionApplied(noteId, correctedSourceText, verification)`
- `correctionFailed(noteId, sourceText, errorMessage, verification)`

## Runtime Constraint
- The current implementation emits `correctionFailed(...)` for parser-side auto-correction attempts instead of writing
  note RAW directly.

## Registration Constraint
- This QObject type is registered via `qmlRegisterType<ContentsStructuredTagValidator>()`.
- The class must remain non-`final` so Qt can wrap it through `QQmlElement<T>`.
