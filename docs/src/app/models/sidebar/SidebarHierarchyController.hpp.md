# `src/app/models/sidebar/SidebarHierarchyController.hpp`

## Role
`SidebarHierarchyController` owns sidebar selection state and active hierarchy bindings.

## Interface Alignment
- Now implements `IActiveHierarchyContextSource`.
- Startup/runtime coordination can still observe activation through the inherited `IActiveHierarchySource` contract
  without depending on the full sidebar implementation type.
- Richer consumers can now depend on the context-level interface to read the active hierarchy controller and active
  note-list model without naming `SidebarHierarchyController` directly.
- The constructor no longer hardcodes a direct-base initializer name for QObject parenting.
  Parent ownership is now applied through `setParent(parent)` in the constructor body, so interface-layer refactors do
  not require a second manual update to the initializer list just to keep the class compiling.
- `activeHierarchyIndex() const noexcept` is now explicitly marked `override`, keeping the declaration aligned with the
  pure virtual interface contract and removing compiler warning noise during generated-moc builds.
- The controller also publishes `activeBindingsChanged()`, a composite change signal for consumers that must
  refresh the active hierarchy index, active hierarchy controller, and active note-list model as one coherent snapshot
  instead of reacting to those bindings one-by-one.
