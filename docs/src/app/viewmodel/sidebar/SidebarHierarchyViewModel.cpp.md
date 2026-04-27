# `src/app/viewmodel/sidebar/SidebarHierarchyViewModel.cpp`

## Implementation Notes
- Constructor now initializes the `IActiveHierarchySource` base.
- Selection-store ownership and provider wiring remain the same; only the active-binding notification surface grew.
- The active hierarchy interface is now consumed by view/panel binding code only; startup runtime loading no longer
  uses sidebar activation as a deferred bootstrap trigger.
- Active hierarchy consumers that need a coherent "toolbar index + hierarchy viewmodel + note-list model" tuple can now
  subscribe to `activeBindingsChanged()`. The implementation emits that composite signal after selection changes and
  after provider mapping refreshes, so QML shells can refresh their entire active-binding snapshot in one step and
  avoid transient cross-domain list ghosts during hierarchy switches.
- `hierarchyViewModelForIndex(...)` and `noteListModelForIndex(...)` now also force
  `QQmlEngine::CppOwnership` on returned `QObject*` bindings before they cross the QML boundary.
  This prevents hierarchy-switch snapshots from handing a member-owned C++ object to the QML garbage collector as if
  it were a disposable JS-owned instance.
