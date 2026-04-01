# `src/app/viewmodel/hierarchy/resources/ResourcesListModel.hpp`

## Responsibility

Declares the dedicated right-panel list model for the Resources hierarchy domain.

## Public Contract

- Exposes the shared list bridge contract used by `ListBarLayout` and editor selection wiring:
  - `itemCount`
  - `currentIndex`
  - `currentNoteId`
  - `currentBodyText`
  - `currentResourceEntry`
  - `searchText`
- Provides note-card compatible roles (`noteId`, `primaryText`, `image`, `imageSource`, `displayDate`,
  `folders`, `tags`) so existing list delegate UI can render without using `LibraryNoteListModel`.
- Adds resource-specific roles (`type`, `format`, `resourcePath`, `resolvedPath`, `source`, `renderMode`,
  `displayName`, `previewText`) for resource-aware rendering paths.
- `currentResourceEntry` exposes the currently selected resource payload as a map, so dedicated file viewers
  can render from list selection without reparsing note bodies.

## Intent

This model intentionally separates resource-list behavior from the generic library note-list model,
so resource-domain changes do not regress library-domain list semantics.
