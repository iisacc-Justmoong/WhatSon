# `src/app/qml/view/content/editor/ContentsDisplayView.qml`

## Responsibility

`ContentsDisplayView.qml` is the desktop editor surface for note contents.
It owns the desktop-only gutter/minimap presentation, the shared source-editing RichText surface, and the resource overlays
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
- `ContentsMinimapSnapshotSupport.js`
- `ContentsResourceViewer.qml`
- `ContentViewLayout.qml`

## Interaction Notes

- The editor stays editable in all supported content modes; the legacy formatted-preview fallback remains disabled.
- Markdown syntax is now treated as raw `.wsnbody` source on the live desktop editor surface.
  The editor no longer prettifies markdown markers into a second display-only grammar while the user is typing.
- Resource cards rendered from `<resource ...>` tags still overlay the editor viewport when the selected note body
  references inline assets.
- Direct `.wsresource` selections still switch the surface to the dedicated in-editor resource viewer.
- Context-menu formatting, keyboard shortcuts, gutter refresh, and minimap snapshot refresh all remain rooted in this
  file.
- The desktop right-click formatting menu now primes the shared selection-controller snapshot on mouse press, before the
  follow-up click/open cycle can let the RichText editor collapse the dragged selection to one fragment.
- Whole-document RichText/plain-text projection now runs through a dedicated `documentPresentationSourceText` snapshot
  instead of a direct `editorText` binding:
  - live typing mutates `editorText` immediately, but `ContentsLogicalTextBridge` and `ContentsTextFormatRenderer`
    now rebuild only after the short presentation timer fires or when blur/note-switch forces a commit
  - `ContentsEditorTypingController.qml` keeps its own incremental plain-text / logical-to-source offset cache between
    those presentation commits, so single-character edits no longer need a whole-document bridge rebuild
  - the live editor still consumes `ContentsTextFormatRenderer.editorSurfaceHtml`, which styles proprietary inline tags
    but leaves markdown syntax literal
  - markdown-aware preview HTML is not rebuilt unless a preview surface explicitly asks for it
  - explicit source-rewrite actions such as inline-format wraps and markdown-list toggles now trigger one immediate
    presentation commit after the source rewrite so the bridge/renderer/minimap state catches up without waiting for
    the idle timer
- While the desktop editor keeps focus, the whole-document presentation timer no longer commits a fresh RichText
  surface back into the live editor.
  Ordinary typing now relies on the incremental typing bridge until blur/explicit flush, preventing per-`120ms`
  surface reinjection from dropping keys, deleting Hangul jamo, or snapping the cursor to a stale logical offset.
- Minimap snapshotting now keeps a cached logical-line group model and only resamples the changed text range through
  `positionToRectangle(...)` during ordinary note edits.
  Full minimap rebuilds remain reserved for layout resets such as width/height changes, route re-entry, or note swaps.
- Gutter line geometry now also reuses that incremental logical-line group cache:
  - `lineDocumentY(...)` and single-line `lineVisualHeight(...)` read from `minimapLineGroups` when that cache is hot
  - the editor keeps the line-geometry refresh path active even when the minimap is visually hidden, so hiding the
    minimap no longer forces gutter math back onto whole-document `positionToRectangle(...)` scans
- Cursor-only and viewport-only minimap updates now reuse cached row geometry; full minimap resampling is limited to
  text/layout changes or an explicit minimap re-enable.
- RAW-safe entity strings stored in source text (`&lt;`, `&gt;`, `&amp;`, etc.) now render as their visible symbols on the
  RichText editor surface instead of showing the literal escape sequences.
- Desktop window shortcuts mirror the selection controller contract for markdown lists:
  - macOS: `Cmd+Shift+7` / `Cmd+Shift+8`
  - Windows/Linux: `Alt+Shift+7` / `Alt+Shift+8`
- Desktop note selection/body echo changes now route through `ContentsEditorSession.requestSyncEditorTextFromSelection(...)`,
  so note switches re-stage the previous note buffer and then bind the next note immediately instead of blocking on an
  immediate-save acceptance path.
- The desktop host now wires `ContentsEditorSession.typingIdleThresholdMs` from
  `contentsView.editorIdleSyncThresholdMs`, so same-note model snapshot apply is gated by an explicit typing-idle
  window instead of raw arrival timing.
- During that non-idle window, same-note mismatched `currentBodyText` snapshots are now dropped and do not rebind the
  live editor buffer.
- Desktop snapshot polling and deferred presentation commit now also share `typingSessionSyncProtected`
  (`ContentsEditorSession.isTypingSessionActive()`) and `pendingBodySave` guards.
  So even if focus churn briefly misreports, timer-driven snapshot/presentation refresh still cannot rebind stale text
  while the user is in an active typing session.
- Desktop editor body writes are now staged into the shared buffered fetch-sync boundary immediately on mutation, while
  actual `.wsnote` synchronization happens later on the controller's recurring `1000ms` fetch turn.
- Desktop note snapshot polling now also pauses while the editor owns input focus.
  This prevents the periodic `currentBodyText` refresh from re-binding a stale same-note body snapshot into the live
  editor while the user is still typing.

## Tests

- Automated test files are not currently present in this repository.
- Regression checklist:
  - Desktop contents must render without reserving any extra bottom partition height.
  - Markdown-style list authoring must not cause the desktop gutter width to oscillate while the editor relayouts.
  - The desktop live editor must keep raw markdown markers (`- `, `1. `, `# `, `> `, `` ``` ``) visible instead of
    replacing them with preview-only glyph/styling.
  - Gutter line numbers and minimap geometry must still align with the live editor surface.
  - Moving the cursor through the desktop editor must update the minimap highlight without rebuilding the whole minimap
    snapshot on every cursor tick.
  - Editing one note paragraph must not force the minimap to call `positionToRectangle(...)` for the entire document;
    only the changed logical-line range should be resampled unless the editor layout itself changed.
  - Desktop typing must not drive markdown-aware preview HTML regeneration on every committed keystroke; only the
    cheaper source-editing surface should be refreshed while preview is disabled.
  - Desktop single-character typing must not rebuild `ContentsLogicalTextBridge` from the whole note on every committed
    key; whole-document logical/source offset regeneration should wait for editor idle, blur, or note switch.
  - While the desktop editor is focused, the presentation timer must not reapply a fresh RichText surface into the
    live `TextEdit`; the whole-document commit should wait for blur or another explicit immediate refresh path.
  - Desktop typing must not perform direct `.wsnote` persistence on every mutation; filesystem sync must flow through
    the buffered fetch boundary instead.
- While the desktop editor is focused, periodic note snapshot polling must not reapply an older same-note
  `currentBodyText` payload into the live editor buffer.
- While the desktop typing session remains active or `pendingBodySave` is true, timer-driven snapshot polling must stay
  paused even if focus flags momentarily flap.
- Desktop Hangul typing must not lose committed syllables or delete partial jamo because of a deferred presentation
  refresh firing mid-edit.
- Desktop gutter/minimap line count must decrease immediately after newline deletion or line-wrap collapse; it must not
  monotonically grow because of stale incremental line-offset state.
  - Desktop cursor restoration after note focus or inline resource insertion must go through one logical cursor path;
    the app must not write conflicting positions into multiple nested editor objects on the same turn.
  - Desktop gutter line-Y queries must prefer cached `minimapLineGroups` geometry instead of falling back to a fresh
    whole-note `positionToRectangle(...)` sweep when the minimap is merely hidden.
  - Resource overlays and dedicated resource viewing must still occupy the editor viewport correctly.
  - `Page` / `Print` mode must keep the external paper-document scroll contract.
  - RAW-safe entity text such as `&lt;bold&gt;` or `Tom &amp; Jerry` must display as visible glyphs in the editor while
    persistence continues to use the source-driven note body path.
  - Desktop markdown list shortcuts (`Cmd+Shift+7/8` on macOS, `Alt+Shift+7/8` on Windows/Linux) must still reach the
    selection controller while the rich-text editor owns focus.
  - Drag-selecting text across multiple paragraphs or mixed inline-style regions, then right-clicking for `Bold` /
    `Italic` / `Underline` / `Highlight`, must still format the entire dragged range instead of only the fragment that
    survives after the context-menu click.
  - Switching desktop note selection while the current note still has a pending staged body must not drop the old note
    text or block the new selection on an immediate-save success path.
