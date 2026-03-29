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

The header now also makes `WhatSonLibraryIndexedState` the private backend note-index dependency.
That narrows the viewmodel's direct storage responsibility to orchestration state instead of three
separate indexed note containers.

It also declares the private note-list item cache helpers that let the viewmodel reuse derived row
data across bucket switches and folder-scope refreshes.

The public invokable surface now includes `reloadNoteMetadataForNoteId(QString)` so external
writers such as the detail panel can force the active library note list to re-mirror `.wsnhead`
metadata for the currently selected note.

## Capability Declaration Contract

The capability-facing methods inherited from `IHierarchyRenameCapability`,
`IHierarchyCrudCapability`, `IHierarchyExpansionCapability`, `IHierarchyReorderCapability`, and
`IHierarchyNoteDropCapability` are declared with explicit `override` markers. That keeps the build
warning-clean and makes accidental signature drift visible at compile time.
