# `src/app/models/editor/input/ContentsEditorTypingController.qml`

## Responsibility

`ContentsEditorTypingController.qml` owns the legacy whole-editor mutation helpers and tag-management shortcuts that
still need `ContentsDisplayView.qml` document-level cursor context.

The live `TextEdit` remains the only ordinary input surface. This controller reacts after native editing has produced a
stable text snapshot; it does not intercept ordinary key events.

## Mutation Model

- Seeds its authoritative plain-text snapshot and logical-to-source offset table from
  `ContentsLogicalTextBridge.logicalText` plus `logicalToSourceOffsets()` when the host commits a new
  `documentPresentationSourceText` snapshot.
- Keeps an incremental live cache of the previous authoritative plain text, logical line-start offsets, and
  logical-to-source offsets between presentation commits.
- Reads committed text from `contentEditor.currentPlainText()` when available, with `getText(0, length)` as the legacy
  surface fallback.
- Computes one contiguous plain-text delta and maps that delta back into RAW `.wsnbody` source offsets.
- The canonical structured text-block path now lives in `ContentsDocumentTextBlockController.qml`: plain text blocks
  commit the `TextEdit` plain text directly as RAW block source, and styled blocks use the tag-preserving replacement
  path.
- Adopts each accepted ordinary edit back into `ContentsLogicalTextBridge` through `adoptIncrementalState(...)` so
  gutter, selection, and cursor helpers stay current without a whole-note rebuild per keystroke.

## Native Input Policy

- The controller does not expose `handlePlainEnterKeyPress(...)`, `handleTagAwareDeleteKeyPress(...)`, or direct raw
  deletion helpers for QML key interception.
- `Enter`, `Backspace`, `Delete`, arrow navigation, selection extension, keyboard repeat, and IME gestures stay on the
  native `TextEdit` path.
- Scheduled cursor restoration never writes through active `inputMethodComposing` or non-empty `preeditText`; the
  pending cursor request is retained until native composition settles.
- Programmatic surface replacement and resource-drop guard turns cannot re-enter this controller as fake user edits.

## Allowed Tag-Management Mutations

The remaining non-literal transformations are source-tag management, not live key interception:

- `queueAgendaShortcutInsertion()` inserts a canonical `<agenda ...><task ...>...</task></agenda>` block at the
  resolved RAW cursor through `ContentsEditorBodyTagInsertionPlanner`.
- `queueCalloutShortcutInsertion()` wraps the active selected RAW range as `<callout>...</callout>` when text is
  selected, otherwise it inserts canonical `<callout>...</callout>` source at the resolved RAW cursor.
- `queueBreakShortcutInsertion()` inserts canonical `</break>` source.
- Typing `[] item` or `[x] item` can canonicalize into agenda/task tags after the native edit is committed.
- A standalone `---` line can canonicalize into `</break>` after the native edit is committed.
- Agenda task Enter handling and legacy whole-editor callout exit handling are delegated to their backends as RAW tag
  mutations after the committed text delta is available.
- Resource drops and clipboard resource imports reuse `insertRawSourceTextAtCursor(...)`, which now delegates raw
  source insertion payload construction to `ContentsEditorBodyTagInsertionPlanner` before applying the next RAW source.

## Disallowed Custom Input

- Markdown list shortcut toggles are not owned by this controller.
- Markdown list Enter continuation is not owned by this controller.
- The controller must not synthesize the next markdown list marker or remove marker-only list lines on Enter.
- Generic key-event handlers must not be reintroduced as a path into ordinary text mutation.

## Persistence Rules

- Accepted source replacements update `view.editorText`, mark local editor authority, schedule persistence, and emit
  `editorTextEdited(...)`.
- Tag-management rewrites refresh presentation immediately because the parser must rematerialize structured blocks from
  the new RAW source.
- Ordinary committed typing uses the incremental live state and does not serialize the rendered RichText surface back
  into `.wsnbody`.

## Regression Checks

- Hangul and other IME composition must mutate `.wsnbody` only after committed text is available, not during preedit.
- Ordinary typing, deletion, and selection replacement must update the correct RAW source span without using key-event
  handlers.
- Markdown marker text such as `- item` and `1. item` should persist literally unless a tag-management rule applies.
- A standalone `---` line should persist canonical `</break>`; longer strings such as `abc---`, `---todo`, and `----`
  should remain literal text.
- Agenda, callout, break, and resource tag insertion must resolve the RAW cursor from the live logical cursor and must
  not nest new wrapper tags inside an existing agenda or callout body.
- The source must not contain list-continuation helpers, raw delete key handlers, or plain Enter key handlers.
