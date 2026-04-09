# `src/app/qml/view/content/editor`

## Status
- Directory mirror generated from the current `src` tree.
- This file is the entry point for the detailed documentation pass of this directory.

## Scope
- Mirrored source directory: `src/app/qml/view/content/editor`
- Child directories: 0
- Child files: 10

## Child Directories
- No child directories.

## Child Files
- `ContentsDisplayView.qml`
- `ContentsEditorSelectionController.qml`
- `ContentsEditorSession.qml`
- `ContentsEditorTypingController.qml`
- `ContentsGutterLayer.qml`
- `ContentsInlineFormatEditor.qml`
- `ContentsMinimapLayer.qml`
- `ContentsMinimapSnapshotSupport.js`
- `MobileContentsDisplayView.qml`
- `ContentsResourceViewer.qml`

## Current Notes

- `ContentsDisplayView.qml` is now the desktop-only editor surface.
- `MobileContentsDisplayView.qml` is the duplicated mobile-only editor surface, so gutter/font behavior is no longer
  gated through one shared desktop/mobile file.
- The shared live editor engine is already `QtQuick.TextEdit` wrapped by `ContentsInlineFormatEditor.qml`; this
  directory no longer depends on an LVRS `LV.TextEditor` implementation.
- The editor, gutter, minimap, and resource overlays now fill the entire `ContentsView` slot.
- `ContentsEditorTypingController.qml` now owns ordinary text-entry mutation routing so typing no longer reserializes
  the whole RichText surface on every edit.
- Desktop/mobile editor views now keep a separate presentation timer for whole-document markdown/RichText refresh, so
  `ContentsTextFormatRenderer` and full minimap resampling no longer run directly on every committed keystroke.
- Desktop/mobile editor views now also keep `documentPresentationSourceText` as the single whole-document presentation
  snapshot. `ContentsLogicalTextBridge` and `ContentsTextFormatRenderer` bind to that snapshot instead of live
  `editorText`, while `ContentsEditorTypingController.qml` carries the incremental plain-text/source-offset state
  between idle commits.
- Desktop/mobile focused editing now also blocks that whole-document presentation timer from pushing a fresh editing
  surface back into the live `TextEdit`.
  Ordinary typing stays on the incremental live cache until blur or another explicit immediate refresh path, which
  prevents dropped keys, Hangul jamo deletion, and stale cursor restoration during active note writing.
- `ContentsEditorTypingController.qml` now also maintains incremental logical line-start offsets and pushes the entire
  live state into `ContentsLogicalTextBridge.adoptIncrementalState(...)`, so bridge consumers stay current without a
  whole-note rebuild per keystroke.
- Desktop/mobile minimap snapshotting now also shares `ContentsMinimapSnapshotSupport.js`, so ordinary note edits only
  re-sample the changed logical-line window instead of walking the full note text through `positionToRectangle(...)`.
- Desktop/mobile line geometry helpers now reuse that same logical-line group cache even when the minimap is hidden, so
  gutter line-Y queries no longer need their own whole-document geometry sweep.
- `ContentsInlineFormatEditor.qml` now also owns the `Qt.inputMethod.update(...)` bridge for cursor, selection, and
  geometry changes, keeping mobile platform text-selection handles and iOS keyboard trackpad gestures aligned with the
  live `TextEdit`.
- Both the wrapper and the selection controller now restore selections with `TextEdit.moveCursorSelection(...)` when an
  active edge matters, so OS-driven selection expansion keeps one continuous anchor instead of degrading to repeated
  fresh sub-selections.
- `ContentsInlineFormatEditor.qml` now also supplements mobile native-input selection with passive touch multi-tap
  handling, restoring double-tap word selection and triple-tap paragraph selection even though the live editor sits
  inside a `Flickable`.
- Mobile editor hosts now opt into native-input priority rules, so active mobile typing defers synchronous persistence,
  pauses note snapshot polling, delays app-driven RichText surface reinjection until the OS input session settles, and
  uses a plain logical-text input surface instead of the RichText editor projection.
- Desktop editor hosts now also pause note snapshot polling while the live editor owns focus, so the periodic
  `currentBodyText` refresh cannot overwrite the active buffer with a stale same-note snapshot mid-typing.
- `MobileContentsDisplayView.qml` also removes the mobile live-editor horizontal inset, so the content view spans the
  full routed mobile width.
- `ContentsEditorSelectionController.qml` now also owns common markdown list shortcuts (`Cmd+Shift+7/8` on macOS,
  `Alt+Shift+7/8` on Windows/Linux) so block-level list toggles stay source-driven as well.
- Markdown list toggles now restore selection/cursor using logical-text lengths from `ContentsLogicalTextBridge`, so
  lines that contain inline tags, escaped entities, or `<resource ...>` tokens no longer jump selection after a list
  rewrite.
- `ContentsEditorSession.qml` no longer serializes note swaps behind pending-body save barriers. It keeps the live editor buffer
  authoritative, re-stages the old note into the buffered fetch-sync controller, and allows the next selected note to
  bind immediately.
- `ContentsEditorSession.qml` now also keeps local editor authority across same-note model echoes, so one successful
  echo cannot immediately reopen the door for the next stale polling snapshot to replace the current note body.
- Editor body persistence is now split into buffered fetch staging vs background completion:
  - `ContentsEditorSession.qml` owns pending/in-flight state only
  - `ContentsEditorSelectionBridge` stays as the QML-facing adapter
  - `file/sync/ContentsEditorIdleSyncController` owns the note-scoped buffered snapshot cache and recurring `1000ms`
    fetch clock
  - `ContentsNoteManagementCoordinator` serializes direct `.wsnote` writes plus open-count/stat follow-up work on the
    downstream management side
  - selection/typing controllers now treat persistence as eventual filesystem sync of the latest buffered note body,
    not as an "all changes must land on this exact idle turn" guarantee
- The RichText editor surface now decodes one safe-entity layer for display, so RAW-preserving source escapes like
  `&lt;` / `&gt;` / `&amp;` render as visible glyphs without changing the canonical note-body source contract.
- Gutter/minimap/editor default geometry now routes through LVRS `gap`, `stroke`, theme-color, and `scaleMetric(...)`
  tokens instead of scattered local editor pixel literals.
- The desktop gutter layout is now hard-clamped to its resolved token width, so markdown-list relayouts cannot make the
  gutter visibly squeeze or rebound while typing.
- Markdown list continuation on `Enter` is also owned by `ContentsEditorTypingController.qml`, so bullet/numbered list
  continuation stays source-driven instead of relying on RichText widget heuristics or the editor's rendered bullet
  glyph representation.
- Empty continued markdown list items now also break back to a plain blank line on `Enter`, so repeated list newlines do
  not get stuck in an endless empty-list state.
- `Page` / `Print` now mount the live RichText editor inside an outer paper-document viewport, so the paper grows with
  the note instead of remaining a fixed-height scaffold.
- The repository no longer operates scripted editor tests; the per-file regression notes in this directory are
  documentation-only behavior contracts.
- Cursor restoration for ordinary typing/focus recovery now routes through the wrapper-level cursor setter instead of
  rewriting `cursorPosition` into the wrapper, `editorItem`, and `inputItem` together.
- The shared editor wrapper no longer depends on its own synthetic IME commit queue for ordinary typing, so a just-typed
  word is no longer left behind a wrapper-local composition flag while the user immediately clicks to move the cursor.
- The shared editor wrapper now also prefers the native `QtQuick.TextEdit` edited-signal / input-method commit path
  instead of maintaining its own synthetic IME commit flag, aligning Hangul composition behavior with standard text
  editors and word processors more closely.

## Intended Detailed Sections
- Module responsibilities and architectural layer
- Internal submodule boundaries
- Cross-directory dependencies
- Runtime ownership and lifecycle rules
- Testing strategy and coverage map
- Known hotspots and refactor priorities
