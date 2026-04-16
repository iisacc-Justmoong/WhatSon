# Hierarchy Regression Checklist

This repository does not operate automated or scripted tests. Use this checklist as manual regression coverage for
hierarchy/sidebar coupling changes.

## Sidebar Mapping
- The sidebar must still resolve Library, Projects, Bookmarks, Tags, Resources, Progress, Event, and Preset through
  their dedicated domain viewmodels after startup wiring completes.
- Reordering the composition-root registration order in `main.cpp` must not change which concrete viewmodel a given
  hierarchy index resolves to.
- An unmapped or out-of-range hierarchy index must still normalize back to the default sidebar domain instead of
  crashing or returning a dangling QObject.

## Active Context Binding
- Changing the active sidebar hierarchy must update the detail panel's current note-list model and current
  note-directory resolver through `DetailPanelCurrentHierarchyBinder`, without hand-written lambda wiring in
  `main.cpp`.
- Detail-panel note context must clear cleanly when the active hierarchy exposes no note-backed list model.
- Restoring Library after visiting a non-note hierarchy must repopulate the detail panel from the active
  `LibraryNoteListModel` and the active hierarchy viewmodel on the next binding turn.

## Startup Coordination
- Startup/runtime code that only depends on `IActiveHierarchySource` must continue to observe the active hierarchy
  index even though the sidebar implementation now also exposes `IActiveHierarchyContextSource`.
- The detail-panel binding path must remain composition-root-only: hierarchy modules should not take a compile-time
  dependency on `DetailPanelViewModel`, and detail-panel internals should not depend on `SidebarHierarchyViewModel`
  directly.
- Changing `SidebarHierarchyViewModel`'s direct interface base must not require a stale constructor initializer update
  just to preserve QObject parent ownership.
  Constructor parenting should remain correct after interface-layer refactors.
