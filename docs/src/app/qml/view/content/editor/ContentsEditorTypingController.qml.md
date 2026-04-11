# `src/app/qml/view/content/editor/ContentsEditorTypingController.qml`

## Responsibility

`ContentsEditorTypingController.qml` owns ordinary editor typing mutations for `ContentsDisplayView.qml`.

This controller exists to keep plain typing separate from inline-format application:
- ordinary typing/backspace/delete/enter/paste
- no whole-document RichText normalization
- direct raw `.wsnbody` source replacement only

## Mutation Model

- Seeds its authoritative plain-text snapshot and logical-to-source offset table from
  `ContentsLogicalTextBridge.logicalText` plus `logicalToSourceOffsets()` only when the host editor view commits a
  new `documentPresentationSourceText` snapshot.
- Between those presentation commits, it keeps an incremental live cache of:
  - the previous authoritative plain text
  - logical line-start offsets
  - the logical-to-source offset table needed to splice `.wsnbody`
- Reads the live edited plain text from `contentEditor.getText(0, length)`.
- Receives committed edit notifications only after the native `TextEdit` input-method session settles, so diffing is
  based on stable plain-text snapshots instead of transient Hangul/Japanese preedit fragments.
- Computes a single contiguous replacement delta (`start`, `previousEnd`, `insertedText`) from those two plain-text
  projections.
- When that delta is a single `Enter` insertion on a markdown list line, the controller now expands the inserted text
  before persistence.
- After list-continuation preprocessing, if the edited logical line becomes exactly `---`, the controller rewrites that
  whole line into the canonical divider source token `</break>` before source persistence.
- The divider shortcut rewrite is line-scoped:
  - it replaces the whole current line range, not only the last typed `-`
  - it does not trigger when `---` appears as part of a longer token (`abc---`, `----`, `---text`)
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
  - marker-only empty list lines do not auto-continue; pressing `Enter` on that empty continued list item now removes
    the list marker and leaves a plain blank line, breaking the repeated list-continuation chain
- Resolves the delta back into source offsets through the incremental live offset cache first, only reseeding from
  `ContentsLogicalTextBridge` when the presentation snapshot is refreshed.
- After each accepted typing mutation, the controller now pushes that incremental logical-text / line-start /
  logical-to-source state back into `ContentsLogicalTextBridge` through `adoptIncrementalState(...)`, so gutter,
  selection, and cursor helpers do not need a whole-note bridge rebuild to stay current while the user is typing.
- Line-start offsets are now rebuilt from the post-edit logical text on every accepted mutation instead of splice-
  merging prior offsets.
  This keeps line-count shrink cases (line-wrap collapse, newline deletion, paragraph merge) from leaving stale line
  starts behind in gutter/minimap geometry.
- Delegates the final source splice to `ContentsTextFormatRenderer.applyPlainTextReplacementToSource(...)`.
- List-continuation cursor restoration now also waits for IME preedit to finish and then moves the cursor through the
  wrapper-level cursor setter only.
  The controller no longer writes the same logical cursor offset into the wrapper, `editorItem`, and `inputItem`
  in parallel.

This means ordinary typing no longer round-trips the entire RichText surface through
`normalizeEditorSurfaceTextToSource(...)`, so fragment scaffold/comment markup from whole-document HTML export is no
longer part of the normal typing path.

## Persistence Rules

- After source replacement, the controller updates `view.editorText`.
- If list continuation inserted extra markdown marker text, the controller also restores the live cursor to the
  continued item start after programmatic surface sync.
- If the divider shortcut rewrote `---` to `</break>`, the controller restores the cursor near the rewritten divider
  line after the programmatic source sync.
- It marks local editor authority before save, matching the existing editor session contract.
- A committed `textEdited` turn is now always treated as user-authoritative typing input.
  The controller no longer drops that turn only because `syncingEditorTextFromModel` happened to still be true from a
  previous model-sync tick.
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
- The only typing-side markdown shortcut exception is a standalone `---` line, which is canonicalized to `</break>`
  before it reaches source persistence.

## Regression Checks

- Markdown-list continuation is now tracked as a documented behavior contract only; this repository no longer maintains
  a dedicated scripted test for it.
- Typing ordinary letters should update raw `.wsnbody` without serializing the whole RichText document.
- Typing ordinary letters must not require a full `ContentsLogicalTextBridge` rebuild or full logical/source offset-map
  regeneration on every committed keystroke.
- Typing ordinary letters should keep `ContentsLogicalTextBridge.logicalLineCount`, line-start offsets, and
  logical-to-source offsets current through incremental adoption instead of waiting for a whole-note bridge refresh.
- Typing a single newline insertion or deletion must keep `logicalLineStartOffsets` and `logicalLineCount` exact in
  both growth and shrink directions; deleting a line break must reduce line count immediately without leaving stale
  extra gutter lines.
- `Space`, `Enter`, `Backspace`, and selection replacement should update the correct source span.
- Direct typing must not leak fragment comment markup such as `<!--StartFragment-->`.
- Typing literal `<bold>` text should persist as literal text, not as an executable inline tag.
- Hangul IME composition must mutate `.wsnbody` only once per committed syllable/result, not once per preedit step.
- The typing controller must not depend on wrapper-owned synthetic IME commit flags; it should only react to the
  native `TextEdit` state that survives after composition settles.
- A user `textEdited` event must not skip local-authority marking/persistence staging solely due to a transient
  model-sync guard bit from a prior same-note sync cycle.
- Post-`Enter` list-continuation cursor restore must not land on the wrong logical offset because multiple nested
  cursor objects were rewritten out of order.
- when `view.deferImmediateEditorPersistence` is enabled, committed typing must not synchronously hit the content store
  on each keystroke
- Typing `- item` or `1. item` should persist the literal markdown marker text in source rather than an already-expanded
  bullet/number glyph representation.
- Pressing `Enter` on an unordered markdown list that is visually rendered with a bullet glyph in the editor must still
  continue with the original source marker (`-`, `*`, or `+`) instead of degrading to a bare newline.
- Pressing `Enter` at the end or middle of a non-empty markdown bullet line should persist `\n- ` / `\n* ` / `\n+ `
  continuation text instead of a bare newline.
- Pressing `Enter` on an empty continued unordered list item (`- ` / `* ` / `+ ` / `• ` with no body text) should
  remove that list marker and leave a plain blank line instead of creating another empty list continuation line.
- Pressing `Enter` immediately after rapid typing on a non-empty markdown list line must still continue that list even
  when the committed edit batch includes both the final typed characters and the newline.
- Pressing the first `Enter` on a newly typed unordered markdown line must still persist a real source marker and the
  continued next marker even when the committed edit batch contains the whole logical line plus newline.
- Pressing `Enter` on a non-empty ordered markdown list line should persist the incremented next prefix (`1.` -> `2.`,
  `3)` -> `4)`).
- Pressing `Enter` on an empty continued ordered markdown list item (`1. ` / `1) ` with no body text) should remove the
  ordered marker and leave a plain blank line instead of extending the empty list.
- Pressing `Enter` on a non-empty ordered line such as `1.hello` or `1)항목` should also continue the list even when
  the original source omitted the usual separator space.
- Typing a standalone `---` line must immediately persist canonical source `</break>` (not literal `---`).
- Typing `---` inside longer text (`abc---`, `---todo`, `----`) must not trigger divider canonicalization.
