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
  before persistence.
- The continuation path is tolerant of short source-sync lag: when fast typing causes the committed delta to include the
  last few typed characters plus the trailing newline together, the controller still treats that edit as one list
  continuation candidate instead of requiring `insertedText === "\n"` exactly.
- The continuation path is also tolerant of first-`Enter` line replacement batches: if the editor commits the whole
  current list line plus the trailing newline before `view.editorText` catches up, the controller still accepts that
  edit as a continuation candidate instead of requiring a pure insertion-only delta.
- The continuation check is source-driven, not renderer-driven:
  - it maps the logical cursor offset back into `view.editorText`
  - it inspects the source-side line prefix there, so `-`, `*`, `+`, and ordered markers still continue correctly even
    though the RichText editor may display unordered markers as a bullet glyph (`•`)
- The continued marker is then written back into source before persistence:
  - unordered list lines (`- ` / `* ` / `+ `) continue with the same marker and indentation
  - ordered list lines (`1. ` / `2) `) continue with the next number, same delimiter, and same indentation
  - ordered list continuation is tolerant of missing post-marker whitespace (`1.text`, `1)text`) and normalizes the
    continued prefix back to a spaced marker (`2. ` / `2) `)
  - numeric literals without a separator such as `3.14` are still excluded from list continuation
  - delayed `Enter` commits that arrive as `item\n` instead of bare `\n` still append the continued marker after that
    newline
  - if the unordered list line is re-sent from the rendered editor with a leading bullet glyph (`• item\n`), the
    controller rewrites that batch back to a source marker (`- item\n`, `* item\n`, `+ item\n`) before persistence so
    the list source does not collapse into display glyphs
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
- Hosts may opt into deferred persistence through `view.deferImmediateEditorPersistence`.
  - when that flag is enabled, ordinary typing skips synchronous `persistEditorTextImmediately(...)`
  - the controller arms `editorSession.scheduleEditorPersistence()` instead, so the OS input session is not blocked by
    note-store writes on every committed mobile keystroke
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
- when `view.deferImmediateEditorPersistence` is enabled, committed typing must not synchronously hit the content store
  on each keystroke
- Typing `- item` or `1. item` should persist the literal markdown marker text in source rather than an already-expanded
  bullet/number glyph representation.
- Pressing `Enter` on an unordered markdown list that is visually rendered with a bullet glyph in the editor must still
  continue with the original source marker (`-`, `*`, or `+`) instead of degrading to a bare newline.
- Pressing `Enter` at the end or middle of a non-empty markdown bullet line should persist `\n- ` / `\n* ` / `\n+ `
  continuation text instead of a bare newline.
- Pressing `Enter` immediately after rapid typing on a non-empty markdown list line must still continue that list even
  when the committed edit batch includes both the final typed characters and the newline.
- Pressing the first `Enter` on a newly typed unordered markdown line must still persist a real source marker and the
  continued next marker even when the committed edit batch contains the whole logical line plus newline.
- Pressing `Enter` on a non-empty ordered markdown list line should persist the incremented next prefix (`1.` -> `2.`,
  `3)` -> `4)`).
- Pressing `Enter` on a non-empty ordered line such as `1.hello` or `1)항목` should also continue the list even when
  the original source omitted the usual separator space.
