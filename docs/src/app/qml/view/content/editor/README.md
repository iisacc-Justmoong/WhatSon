# `src/app/qml/view/content/editor`

## Status
- Directory mirror generated from the current `src` tree.
- This file is the entry point for the detailed documentation pass of this directory.

## Scope
- Mirrored source directory: `src/app/qml/view/content/editor`
- Child directories: 0
- Child files: 9

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
- `MobileContentsDisplayView.qml` also removes the mobile live-editor horizontal inset, so the content view spans the
  full routed mobile width.
- `ContentsEditorSelectionController.qml` now also owns common markdown list shortcuts (`Cmd+Shift+7/8` on macOS,
  `Alt+Shift+7/8` on Windows/Linux) so block-level list toggles stay source-driven as well.
- Markdown list toggles now restore selection/cursor using logical-text lengths from `ContentsLogicalTextBridge`, so
  lines that contain inline tags, escaped entities, or `<resource ...>` tokens no longer jump selection after a list
  rewrite.
- `ContentsEditorSession.qml` now defers note swaps behind pending-body flushes, so a failed immediate save cannot
  silently discard the old note buffer when the user changes selection.
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

## Intended Detailed Sections
- Module responsibilities and architectural layer
- Internal submodule boundaries
- Cross-directory dependencies
- Runtime ownership and lifecycle rules
- Testing strategy and coverage map
- Known hotspots and refactor priorities
