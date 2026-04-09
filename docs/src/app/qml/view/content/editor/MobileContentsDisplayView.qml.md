# `src/app/qml/view/content/editor/MobileContentsDisplayView.qml`

## Responsibility

`MobileContentsDisplayView.qml` is the mobile-only contents editor surface.
It duplicates the desktop editor contract where needed, while keeping mobile-specific font sizing and gutter
suppression local to this file.

## Mobile Policy

- Editor text now follows the same `12px` host policy as desktop.
- Gutter chrome remains disabled on mobile.
- The content surface now fills the full available slot.
- The live mobile content surface now removes its left/right editor inset, so the content view reaches the full routed
  mobile width.
- Mobile typing now prioritizes OS-native input behavior over live app-side RichText resync:
  - the editor enables `preferNativeInputHandling`
  - the live mobile input surface still consumes `ContentsLogicalTextBridge.logicalText`, but that bridge now follows
    an idle/blur-scoped `documentPresentationSourceText` snapshot instead of rebuilding directly from every
    `editorText` mutation
  - synchronous per-keystroke persistence is deferred to the existing debounce session path
  - periodic note snapshot polling pauses while the editor keeps input focus
- The mobile presentation timer now also refuses to commit a whole-document bridge/surface refresh while the live
  editor still owns focus.
  Ordinary typing stays on the incremental live cache until blur/explicit flush so the OS input session is not
  interrupted by an app-side resync.
- Mobile typing diffs now run from the shared incremental typing cache rather than forcing
  `ContentsLogicalTextBridge` to rebuild for each committed character, and the markdown-aware preview renderer remains
  completely skipped while the live source editor is active and no formatted preview is visible.
- Mobile line geometry now also follows the shared incremental logical-line group cache:
  - single-line gutter/minimap geometry queries reuse `minimapLineGroups` instead of rebuilding whole-document
    line-rectangle caches
  - the geometry refresh path stays active even while the parent route hides the minimap, so hiding the minimap does
    not push line-Y math back onto full-document sampling
- Mobile now also depends on the shared editor wrapper's `Qt.inputMethod.update(...)` path so iOS can re-query current
  selection/cursor geometry while keyboard trackpad gestures are active.
- Mobile also depends on the shared wrapper/controller preserving the active selection edge with
  `TextEdit.moveCursorSelection(...)` whenever the app has to restore a selection programmatically.
- Mobile also depends on the shared wrapper's passive touch multi-tap selection support:
  - double-tap reselects the touched word
  - triple-tap expands to the surrounding paragraph
- Cursor-only and scroll-only minimap updates now stay on cached geometry, and when the parent route disables the
  minimap the mobile surface skips minimap sampling entirely.
- Minimap snapshotting now shares the same incremental logical-line cache as desktop.
  Ordinary body edits only resample the changed text range through `positionToRectangle(...)`, while route/layout
  resets still trigger a full minimap rebuild.
- Minimap visibility is still controlled by the parent route/layout contract.
- Mobile shares the same LVRS tokenized editor/gutter/minimap metric defaults and editor typography baseline as desktop.
- Print-paper/resource card border thickness also follows `LV.Theme.strokeThin`.
- `Page` / `Print` mode still scroll the outer paper-document viewport instead of a fixed-height nested editor.

## Ownership

- `ContentViewLayout.qml` selects this file only when the LVRS window reports a mobile platform.
- `MobileHierarchyPage.qml` reaches this file through `ContentViewLayout.qml`.
- The editor session, typing controller, selection controller, renderer, and resource-viewer collaborators stay aligned
  with the desktop implementation.
- `ContentsMinimapSnapshotSupport.js` is shared with desktop so the minimap diff/range-splice policy stays identical
  across both editor surfaces.
- RAW-safe entity strings stored in source text (`&lt;`, `&gt;`, `&amp;`, etc.) now render as their visible symbols on the
  mobile RichText editor surface as well.
- While the mobile editor is focused, app-side note snapshot refresh does not preempt the live native input session.
- Inline styling on mobile remains source-driven, but the live typing surface is now plain logical text rather than the
  RichText projection.
- That logical text now preserves raw markdown markers instead of replacing unordered-list markers with `•`.
- Mobile keeps the same window-level markdown list shortcuts as desktop when a hardware keyboard is present:
  - macOS: `Cmd+Shift+7` / `Cmd+Shift+8`
  - Windows/Linux: `Alt+Shift+7` / `Alt+Shift+8`
- Mobile note selection/body echo changes now also route through `ContentsEditorSession.requestSyncEditorTextFromSelection(...)`,
  so the old note buffer stays staged in the fetch-sync controller while the newly selected note binds immediately.
- Mobile editor body writes now also stage immediately into the shared buffered fetch-sync boundary, while actual
  `.wsnote` synchronization waits for a later recurring fetch turn instead of an exact idle/flush moment.

## Tests

- Automated test files are not currently present in this repository.
- Regression checklist:
  - Mobile contents must render without reserving any extra bottom partition height.
  - Mobile content view text and resource cards must reach the full routed mobile width without reintroducing left/right
    editor padding.
  - Mobile editor text must stay aligned with the desktop `12px` baseline.
  - Mobile note editing, resource rendering, and deferred persistence must stay aligned with the desktop editor flow
    except for the mobile-only native-input priority rules.
  - Mobile typing must not trigger whole-document markdown/RichText presentation refresh on every committed keystroke;
    the markdown preview renderer should stay idle while the source editor remains active and preview is hidden.
  - Mobile single-character typing must also avoid whole-note `ContentsLogicalTextBridge` regeneration until the editor
    goes idle, loses focus, or switches notes.
  - While the mobile editor is focused, the presentation timer must not reapply the whole-document editing surface back
    into the live `TextEdit`; the commit must wait for blur or another explicit immediate refresh path.
  - Mobile typing must not perform direct `.wsnote` persistence on every mutation; filesystem sync must flow through
    the buffered fetch boundary instead.
  - Mobile hidden-minimap states must still keep line geometry current through the incremental line-group cache rather
    than re-running whole-note text-geometry sampling for gutter helpers.
  - Mobile body edits must not force full-document minimap text-geometry sampling; only the changed logical-line
    range should be re-sampled unless the editor layout changed.
  - The mobile editor must render `ContentsLogicalTextBridge.logicalText` as the active input text so double-tap
    selection, repeat backspace, and IME composition remain OS-driven.
  - The mobile live input text must keep raw markdown markers visible instead of replacing list markers with `•`.
  - iOS spacebar cursor-drag and other OS cursor-tracking gestures must not trigger full minimap resampling or any
    app-side RichText surface reinjection while native input handling is active.
  - iOS keyboard-based selection gestures must continue to update the selected range after cursor/selection/scroll
    changes because the wrapper keeps `Qt.inputMethod.update(...)` in sync with the live `TextEdit`.
  - iOS keyboard-based range selection must keep extending/shrinking one continuous range; the app must not collapse
    the existing selection down to only the most recently traversed text fragment.
  - iOS touch double-tap must still select the touched word while native input handling is active.
  - iOS touch triple-tap must still select the surrounding paragraph while native input handling is active.
  - While the mobile editor is focused, note snapshot polling must not re-sync the current note body back into the live
    input surface.
  - Mobile Hangul typing must not split jamo or jump the cursor because of app-driven surface reinjection during the
    active input session.
  - Mobile cursor restoration after note focus or inline resource insertion must use one wrapper-level cursor path
    rather than rewriting the same cursor offset into multiple nested editor objects.
  - RAW-safe entity text such as `&lt;bold&gt;` or `Tom &amp; Jerry` must display as visible glyphs on mobile while the
    source-driven persistence path remains unchanged.
  - Mobile `Page` / `Print` mode must keep the outer paper-document scroll contract.
  - Mobile hardware-keyboard markdown list shortcuts (`Cmd+Shift+7/8` on macOS, `Alt+Shift+7/8` on Windows/Linux)
    must stay aligned with the desktop markdown list behavior.
  - Switching mobile note selection while the current note still has a pending staged body must not drop the old note
    text or block the new selection on an immediate-save success path.
