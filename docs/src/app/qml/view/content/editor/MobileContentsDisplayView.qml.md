# `src/app/qml/view/content/editor/MobileContentsDisplayView.qml`

## Responsibility

`MobileContentsDisplayView.qml` is the mobile-only duplicate of the editor surface.

The repository now keeps desktop and mobile editor code in separate QML files so mobile gutter suppression cannot
disable desktop gutter chrome through shared conditionals.

## Mobile Policy

- editor text renders at `14px` (`desktop 12px + 2px`)
- gutter chrome is always disabled
- minimap visibility is still controlled by the parent route/layout contract
- note editing, resource rendering, gutter/minimap geometry helpers, and persistence flow remain functionally aligned
  with the desktop editor
- page/print rendering must follow the same external paper-document scroll contract as desktop:
  - the live editor runs with `externalScroll: true`
  - the mobile page/print viewport scrolls the paper document surface, not a nested fixed-height editor frame
  - `printDocumentPageCount` expands the paper surface as the note grows
- new-note initial typing must keep local editor authority until the note list model echoes the same body text back;
  mobile must not accept a stale empty snapshot during the first few keystrokes
- the duplicated helper functions (`buildFallbackMinimapVisualRows()`, `buildVisibleGutterLineEntries()`,
  `documentOccupiedBottomY()`, `normalizeResourceFormat()`) must preserve the same explicit return contracts as the
  desktop file even though mobile keeps gutter chrome hidden

## Ownership

- `ContentViewLayout.qml` selects this file only when the LVRS window reports a mobile platform.
- `MobileHierarchyPage.qml` reaches this file through `ContentViewLayout.qml`; it no longer relies on desktop editor
  code with mobile suppression flags layered on top.
- `ContentsInlineFormatEditor.qml`, `ContentsEditorTypingController.qml`, `ContentsEditorSelectionController.qml`, and
  the renderer/bridge collaborators keep the same contracts as the desktop editor file.

## Regression Checks

- This repository no longer maintains a dedicated scripted test for the mobile page/print editor surface.
- mobile editor surfaces must not render or reserve gutter width
- mobile editor text must render at `14px`
- desktop gutter behavior must not depend on this file
- mobile note editing, resource rendering, and persistence must stay aligned with the desktop editor flow
- mobile page/print mode must keep the outer paper-document scroll contract instead of falling back to a nested editor
  `Flickable`
