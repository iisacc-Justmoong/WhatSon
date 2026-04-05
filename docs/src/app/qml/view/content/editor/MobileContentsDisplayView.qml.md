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
- `Page` / `Print` mode still scroll the outer paper-document viewport instead of a fixed-height nested editor.

## Ownership

- `ContentViewLayout.qml` selects this file only when the LVRS window reports a mobile platform.
- `MobileHierarchyPage.qml` reaches this file through `ContentViewLayout.qml`.
- The editor session, typing controller, selection controller, renderer, and resource-viewer collaborators stay aligned
  with the desktop implementation.

## Tests

- Automated test files are not currently present in this repository.
- Regression checklist:
  - Mobile contents must render without reserving any extra bottom partition height.
  - Mobile editor text must still render at `14px`.
  - Mobile note editing, resource rendering, and persistence must stay aligned with the desktop editor flow.
  - Mobile `Page` / `Print` mode must keep the outer paper-document scroll contract.
