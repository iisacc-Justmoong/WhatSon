# `src/app/qml/view/content/editor/ContentsDisplayView.qml`

## Responsibility

`ContentsDisplayView.qml` is the desktop editor surface for note contents.
It owns the desktop-only gutter/minimap presentation, the shared rich-text editing surface, and the resource overlays
that sit directly inside the editor viewport.

## Current Layout Contract

- The editor surface now fills the entire available content slot.
- The root surface keeps `displayColor` as the only background fill for the editor area.
- The desktop gutter remains visible when the selected surface is an editable note body.
- The desktop gutter width is now hard-clamped to the tokenized gutter width, so editor-content relayout cannot
  compress or expand the gutter during live typing.
- The minimap remains desktop-only and is mounted beside the editor viewport.
- Editor/gutter/minimap default geometry now routes through LVRS theme tokens:
  - editor font size and horizontal/bottom insets use `LV.Theme.scaleMetric(...)` / `LV.Theme.gap16`
  - gutter rails and width use `LV.Theme.gap2/gap4` plus `LV.Theme.scaleMetric(10/18/40/74/26/14)`
  - minimap track width/inset and viewport minimum height use `LV.Theme.gap8` and `LV.Theme.scaleMetric(36/56/28)`
  - print-paper/resource card border thickness uses `LV.Theme.strokeThin`
- `Page` / `Print` mode still route the live editor through the outer paper-document viewport so scrolling owns the
  paper surface rather than a fixed-height nested editor.

## Key Collaborators

- `ContentsInlineFormatEditor.qml`
- `ContentsEditorSelectionController.qml`
- `ContentsEditorTypingController.qml`
- `ContentsEditorSession.qml`
- `ContentsGutterLayer.qml`
- `ContentsMinimapLayer.qml`
- `ContentsResourceViewer.qml`
- `ContentViewLayout.qml`

## Interaction Notes

- The editor stays editable in all supported content modes; the legacy formatted-preview fallback remains disabled.
- Resource cards rendered from `<resource ...>` tags still overlay the editor viewport when the selected note body
  references inline assets.
- Direct `.wsresource` selections still switch the surface to the dedicated in-editor resource viewer.
- Context-menu formatting, keyboard shortcuts, gutter refresh, and minimap snapshot refresh all remain rooted in this
  file.
- Whole-document RichText projection now runs through a short presentation timer instead of a direct `editorText`
  binding:
  - live typing keeps `ContentsLogicalTextBridge` current for source/plain-text diffing
  - `ContentsTextFormatRenderer.sourceText`, the cached `renderedEditorText`, and the full minimap snapshot refresh
    are committed after a short idle window or immediately when focus leaves / note selection changes
- Cursor-only and viewport-only minimap updates now reuse cached row geometry; full minimap resampling is limited to
  text/layout changes or an explicit minimap re-enable.
- RAW-safe entity strings stored in source text (`&lt;`, `&gt;`, `&amp;`, etc.) now render as their visible symbols on the
  RichText editor surface instead of showing the literal escape sequences.
- Desktop window shortcuts mirror the selection controller contract for markdown lists:
  - macOS: `Cmd+Shift+7` / `Cmd+Shift+8`
  - Windows/Linux: `Alt+Shift+7` / `Alt+Shift+8`
- Desktop note selection/body echo changes now route through `ContentsEditorSession.requestSyncEditorTextFromSelection(...)`,
  so a failed pending-body flush no longer lets a note switch silently overwrite the still-unsaved editor buffer.

## Tests

- Automated test files are not currently present in this repository.
- Regression checklist:
  - Desktop contents must render without reserving any extra bottom partition height.
  - Markdown-style list authoring must not cause the desktop gutter width to oscillate while the editor relayouts.
  - Gutter line numbers and minimap geometry must still align with the live editor surface.
  - Moving the cursor through the desktop editor must update the minimap highlight without rebuilding the whole minimap
    snapshot on every cursor tick.
  - Desktop typing must not drive `ContentsTextFormatRenderer` and whole-editor RichText surface reinjection on every
    committed keystroke; those presentation updates should land after the short idle debounce or on blur/note switch.
  - Resource overlays and dedicated resource viewing must still occupy the editor viewport correctly.
  - `Page` / `Print` mode must keep the external paper-document scroll contract.
  - RAW-safe entity text such as `&lt;bold&gt;` or `Tom &amp; Jerry` must display as visible glyphs in the editor while
    persistence continues to use the source-driven note body path.
  - Desktop markdown list shortcuts (`Cmd+Shift+7/8` on macOS, `Alt+Shift+7/8` on Windows/Linux) must still reach the
    selection controller while the rich-text editor owns focus.
  - Switching desktop note selection while the current note still has a pending unsaved body must either flush that body
    first or defer the editor swap; it must not drop the unsaved text.
