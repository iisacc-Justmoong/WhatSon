# `src/app/viewmodel/detailPanel/DetailPanelViewModel.hpp`

## Responsibility
`DetailPanelViewModel` owns the active detail-panel page state, toolbar selection state, and the three detail-local hierarchy selector copies used by the properties form.

## Owned Objects
- `DetailContentSectionViewModel` instances for:
  - `properties`
  - `fileStat`
  - `insert`
  - `fileHistory`
  - `layer`
  - `help`
- `DetailHierarchySelectionViewModel` instances for:
  - `projectSelectionViewModel`
  - `bookmarkSelectionViewModel`
  - `progressSelectionViewModel`

## Public Wiring Surface
- `activeContentViewModel`
- `activeStateName`
- `toolbarItems`
- `projectSelectionViewModel`
- `bookmarkSelectionViewModel`
- `progressSelectionViewModel`

## Dependency Direction
The detail panel no longer binds QML selectors directly to the sidebar hierarchy viewmodels.
Instead, C++ injects those hierarchy viewmodels as read-only sources into the owned selector-copy objects.
