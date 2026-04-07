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
  - the live mobile input surface now binds to `ContentsLogicalTextBridge.logicalText` and stays in `TextEdit.PlainText`
    instead of editing the RichText projection directly
  - synchronous per-keystroke persistence is deferred to the existing debounce session path
  - periodic note snapshot polling pauses while the editor keeps input focus
- Mobile still keeps the source/plain-text bridge current for typing diffs, but the expensive markdown-aware
  `ContentsTextFormatRenderer` refresh plus full minimap snapshot now commit on a short idle timer or when the native
  input session settles.
- Mobile now also depends on the shared editor wrapper's `Qt.inputMethod.update(...)` path so iOS can re-query current
  selection/cursor geometry while keyboard trackpad gestures are active.
- Mobile also depends on the shared wrapper/controller preserving the active selection edge with
  `TextEdit.moveCursorSelection(...)` whenever the app has to restore a selection programmatically.
- Mobile also depends on the shared wrapper's passive touch multi-tap selection support:
  - double-tap reselects the touched word
  - triple-tap expands to the surrounding paragraph
- Cursor-only and scroll-only minimap updates now stay on cached geometry, and when the parent route disables the
  minimap the mobile surface skips minimap sampling entirely.
- Minimap visibility is still controlled by the parent route/layout contract.
- Mobile shares the same LVRS tokenized editor/gutter/minimap metric defaults and editor typography baseline as desktop.
- Print-paper/resource card border thickness also follows `LV.Theme.strokeThin`.
- `Page` / `Print` mode still scroll the outer paper-document viewport instead of a fixed-height nested editor.

## Ownership

- `ContentViewLayout.qml` selects this file only when the LVRS window reports a mobile platform.
- `MobileHierarchyPage.qml` reaches this file through `ContentViewLayout.qml`.
- The editor session, typing controller, selection controller, renderer, and resource-viewer collaborators stay aligned
  with the desktop implementation.
- RAW-safe entity strings stored in source text (`&lt;`, `&gt;`, `&amp;`, etc.) now render as their visible symbols on the
  mobile RichText editor surface as well.
- While the mobile editor is focused, app-side note snapshot refresh does not preempt the live native input session.
- Inline styling on mobile remains source-driven, but the live typing surface is now plain logical text rather than the
  RichText projection.
- Mobile keeps the same window-level markdown list shortcuts as desktop when a hardware keyboard is present:
  - macOS: `Cmd+Shift+7` / `Cmd+Shift+8`
  - Windows/Linux: `Alt+Shift+7` / `Alt+Shift+8`
- Mobile note selection/body echo changes now also route through `ContentsEditorSession.requestSyncEditorTextFromSelection(...)`,
  so a failed pending-body flush cannot silently replace the current unsaved editor buffer with the newly selected note.

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
    the renderer/minimap path should settle after the short idle timer or when focus leaves.
  - The mobile editor must render `ContentsLogicalTextBridge.logicalText` as the active input text so double-tap
    selection, repeat backspace, and IME composition remain OS-driven.
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
  - RAW-safe entity text such as `&lt;bold&gt;` or `Tom &amp; Jerry` must display as visible glyphs on mobile while the
    source-driven persistence path remains unchanged.
  - Mobile `Page` / `Print` mode must keep the outer paper-document scroll contract.
  - Mobile hardware-keyboard markdown list shortcuts (`Cmd+Shift+7/8` on macOS, `Alt+Shift+7/8` on Windows/Linux)
    must stay aligned with the desktop markdown list behavior.
  - Switching mobile note selection while the current note still has a pending unsaved body must either flush that body
    first or defer the editor swap; it must not drop the unsaved text.
