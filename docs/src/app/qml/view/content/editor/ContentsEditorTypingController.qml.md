# `src/app/qml/view/content/editor/ContentsEditorTypingController.qml`

## Responsibility

`ContentsEditorTypingController.qml` owns ordinary editor typing mutations for `ContentsDisplayView.qml`.

This controller exists to keep plain typing separate from inline-format application:
- ordinary typing/backspace/delete/enter/paste
- no whole-document RichText normalization
- direct raw `.wsnbody` source replacement only

## Mutation Model

- Reads the authoritative previous plain text from `ContentsLogicalTextBridge.logicalText`.
- Reads the live edited plain text from `contentEditor.getText(0, length)`.
- Computes a single contiguous replacement delta (`start`, `previousEnd`, `insertedText`) from those two plain-text
  projections.
- Resolves the delta back into source offsets through `ContentsLogicalTextBridge.sourceOffsetForLogicalOffset(...)`.
- Delegates the final source splice to `ContentsTextFormatRenderer.applyPlainTextReplacementToSource(...)`.

This means ordinary typing no longer round-trips the entire RichText surface through
`normalizeEditorSurfaceTextToSource(...)`, so fragment scaffold/comment markup from whole-document HTML export is no
longer part of the normal typing path.

## Persistence Rules

- After source replacement, the controller updates `view.editorText`.
- It marks local editor authority before save, matching the existing editor session contract.
- It tries `view.persistEditorTextImmediately(...)` first.
- If immediate persistence is unavailable or fails, it falls back to `editorSession.scheduleEditorPersistence()`.
- The host view still emits `editorTextEdited(...)` for the broader editor-shell lifecycle.

## Normalization Rules

- Plain-text comparison normalizes:
  - CRLF / CR -> LF
  - `U+2028` / `U+2029` -> LF
  - NBSP -> regular space
- The source-side splice path escapes literal text before inserting it into `.wsnbody`, so ordinary typing cannot
  inject raw inline tags accidentally.

## Regression Checks

- Typing ordinary letters should update raw `.wsnbody` without serializing the whole RichText document.
- `Space`, `Enter`, `Backspace`, and selection replacement should update the correct source span.
- Direct typing must not leak fragment comment markup such as `<!--StartFragment-->`.
- Typing literal `<bold>` text should persist as literal text, not as an executable inline tag.
