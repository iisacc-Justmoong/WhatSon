# `src/app/qml/view/content/editor/MobileContentsDisplayView.qml`

## Responsibility

`MobileContentsDisplayView.qml` is the mobile-only contents editor surface.
It duplicates the desktop editor contract where needed, while keeping mobile-specific font sizing and gutter
suppression local to this file.

## Mobile Policy

- Editor text renders at `14px` (`desktop 12px + 2px`).
- Gutter chrome remains disabled on mobile.
- The content surface now fills the full available slot.
- Minimap visibility is still controlled by the parent route/layout contract.
- Mobile shares the same LVRS tokenized editor/gutter/minimap metric defaults as desktop, with the mobile font bump
  layered on top through `editorMobileFontPixelSizeOffset`.
- Print-paper/resource card border thickness also follows `LV.Theme.strokeThin`.
- `Page` / `Print` mode still scroll the outer paper-document viewport instead of a fixed-height nested editor.

## Ownership

- `ContentViewLayout.qml` selects this file only when the LVRS window reports a mobile platform.
- `MobileHierarchyPage.qml` reaches this file through `ContentViewLayout.qml`.
- The editor session, typing controller, selection controller, renderer, and resource-viewer collaborators stay aligned
  with the desktop implementation.
- RAW-safe entity strings stored in source text (`&lt;`, `&gt;`, `&amp;`, etc.) now render as their visible symbols on the
  mobile RichText editor surface as well.
- Mobile keeps the same window-level markdown list shortcuts as desktop when a hardware keyboard is present:
  - macOS: `Cmd+Shift+7` / `Cmd+Shift+8`
  - Windows/Linux: `Alt+Shift+7` / `Alt+Shift+8`

## Tests

- Automated test files are not currently present in this repository.
- Regression checklist:
  - Mobile contents must render without reserving any extra bottom partition height.
  - Mobile editor text must still render at `14px`.
  - Mobile note editing, resource rendering, and persistence must stay aligned with the desktop editor flow.
  - RAW-safe entity text such as `&lt;bold&gt;` or `Tom &amp; Jerry` must display as visible glyphs on mobile while the
    source-driven persistence path remains unchanged.
  - Mobile `Page` / `Print` mode must keep the outer paper-document scroll contract.
  - Mobile hardware-keyboard markdown list shortcuts (`Cmd+Shift+7/8` on macOS, `Alt+Shift+7/8` on Windows/Linux)
    must stay aligned with the desktop markdown list behavior.
