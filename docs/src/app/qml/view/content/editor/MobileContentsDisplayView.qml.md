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

## Tests

- Automated test files are not currently present in this repository.
- Regression checklist:
  - Mobile contents must render without reserving any extra bottom partition height.
  - Mobile content view text and resource cards must reach the full routed mobile width without reintroducing left/right
    editor padding.
  - Mobile editor text must stay aligned with the desktop `12px` baseline.
  - Mobile note editing, resource rendering, and deferred persistence must stay aligned with the desktop editor flow
    except for the mobile-only native-input priority rules.
  - The mobile editor must render `ContentsLogicalTextBridge.logicalText` as the active input text so double-tap
    selection, repeat backspace, and IME composition remain OS-driven.
  - While the mobile editor is focused, note snapshot polling must not re-sync the current note body back into the live
    input surface.
  - Mobile Hangul typing must not split jamo or jump the cursor because of app-driven surface reinjection during the
    active input session.
  - RAW-safe entity text such as `&lt;bold&gt;` or `Tom &amp; Jerry` must display as visible glyphs on mobile while the
    source-driven persistence path remains unchanged.
  - Mobile `Page` / `Print` mode must keep the outer paper-document scroll contract.
  - Mobile hardware-keyboard markdown list shortcuts (`Cmd+Shift+7/8` on macOS, `Alt+Shift+7/8` on Windows/Linux)
    must stay aligned with the desktop markdown list behavior.
