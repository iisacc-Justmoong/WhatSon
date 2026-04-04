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
- Receives committed edit notifications only after IME composition settles, so diffing is based on stable plain-text
  snapshots instead of transient Hangul/Japanese preedit fragments.
- Computes a single contiguous replacement delta (`start`, `previousEnd`, `insertedText`) from those two plain-text
  projections.
- When that delta is a single `Enter` insertion on a markdown list line, the controller now expands the inserted text
  before persistence:
  - unordered list lines (`- ` / `* ` / `+ `) continue with the same marker and indentation
  - ordered list lines (`1. ` / `2) `) continue with the next number, same delimiter, and same indentation
  - marker-only empty list lines do not auto-continue, so the controller does not force an endless list
- Resolves the delta back into source offsets through `ContentsLogicalTextBridge.sourceOffsetForLogicalOffset(...)`.
- Delegates the final source splice to `ContentsTextFormatRenderer.applyPlainTextReplacementToSource(...)`.

This means ordinary typing no longer round-trips the entire RichText surface through
`normalizeEditorSurfaceTextToSource(...)`, so fragment scaffold/comment markup from whole-document HTML export is no
longer part of the normal typing path.

## Persistence Rules

- After source replacement, the controller updates `view.editorText`.
- If list continuation inserted extra markdown marker text, the controller also restores the live cursor to the
  continued item start after programmatic surface sync.
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
- Markdown-style prefixes such as `1. `, `- `, `# `, `> `, and `` ``` `` still enter the source as literal typed text;
  the renderer layer is responsible for their RichText presentation after the source update lands.

## Regression Checks

- Markdown-list continuation is now tracked as a documented behavior contract only; this repository no longer maintains
  a dedicated scripted test for it.
- Typing ordinary letters should update raw `.wsnbody` without serializing the whole RichText document.
- `Space`, `Enter`, `Backspace`, and selection replacement should update the correct source span.
- Direct typing must not leak fragment comment markup such as `<!--StartFragment-->`.
- Typing literal `<bold>` text should persist as literal text, not as an executable inline tag.
- Hangul IME composition must mutate `.wsnbody` only once per committed syllable/result, not once per preedit step.
- Typing `- item` or `1. item` should persist the literal markdown marker text in source rather than an already-expanded
  bullet/number glyph representation.
- Pressing `Enter` at the end or middle of a non-empty markdown bullet line should persist `\n- ` / `\n* ` / `\n+ `
  continuation text instead of a bare newline.
- Pressing `Enter` on a non-empty ordered markdown list line should persist the incremented next prefix (`1.` -> `2.`,
  `3)` -> `4)`).
