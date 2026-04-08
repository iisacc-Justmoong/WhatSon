# `src/app/viewmodel/hierarchy/library`

## Role
This directory contains the library hierarchy runtime state, note-list state, and the library-specific viewmodel surfaces consumed by QML.

The library hierarchy remains the richest hierarchy domain in the repository because it is tied directly to note creation, folder mutation, selection, and list rendering.

## Important Types
- `LibraryHierarchyModel`: in-memory hierarchy state.
- `LibraryHierarchyViewModel`: broad library-facing hierarchy viewmodel with both read state and capability implementations.
- `LibraryNoteListModel`: note-list surface for the currently active library selection.
- `LibraryNoteMutationViewModel`: narrower facade used by shortcut and panel flows that only need note mutation commands.

## Architectural Direction
The current direction is to stop exposing the full library hierarchy surface to every consumer.
- Read flows should depend on `IHierarchyViewModel`.
- Write flows should depend on explicit capabilities or `LibraryNoteMutationViewModel`.
- Local note mutation flows now also prefer incremental indexed-state updates over full snapshot replacement, so this
  directory is part of the runtime performance boundary for both the note list and the calendar projections.

This directory is therefore one of the main places where the repository's MVVM decomposition is being pushed forward.
