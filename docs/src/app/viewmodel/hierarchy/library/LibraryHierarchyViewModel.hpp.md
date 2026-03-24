# `src/app/viewmodel/hierarchy/library/LibraryHierarchyViewModel.hpp`

## Responsibility

This header declares the main orchestration viewmodel for the library hierarchy domain.

## UUID-Oriented Selection Contract

Folder selection scope no longer stores only a path key. It now tracks the selected folder UUID so
note filtering, drag-and-drop assignment, and hierarchy mutations can resolve the intended folder
even when the visible path changes.

## Important Private Helpers

- `folderUuidForIndex(...)`: maps a UI row to the persisted folder UUID.
- folder-scope helpers use UUIDs first and path labels only as compatibility fallbacks.

## Architectural Role

The viewmodel remains the boundary between:

- QML/LVRS hierarchy interactions
- runtime note filtering
- persistent folder mutation services

The UUID change reduces accidental coupling between visible path strings and semantic folder
identity, but the class is still the central coordinator for the library hierarchy domain.
