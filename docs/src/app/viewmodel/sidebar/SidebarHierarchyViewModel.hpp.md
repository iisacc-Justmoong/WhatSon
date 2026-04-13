# `src/app/viewmodel/sidebar/SidebarHierarchyViewModel.hpp`

## Role
`SidebarHierarchyViewModel` owns sidebar selection state and active hierarchy bindings.

## Interface Alignment
- Now implements `IActiveHierarchySource`.
- This allows startup runtime coordination to observe activation changes without depending on the full sidebar implementation type.
- `activeHierarchyIndex() const noexcept` is now explicitly marked `override`, keeping the declaration aligned with the
  pure virtual interface contract and removing compiler warning noise during generated-moc builds.
- The viewmodel also publishes `activeBindingsChanged()`, a composite change signal for consumers that must
  refresh the active hierarchy index, active hierarchy viewmodel, and active note-list model as one coherent snapshot
  instead of reacting to those bindings one-by-one.
