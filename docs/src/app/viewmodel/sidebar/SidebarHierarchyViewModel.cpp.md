# `src/app/viewmodel/sidebar/SidebarHierarchyViewModel.cpp`

## Implementation Notes
- Constructor now initializes the `IActiveHierarchySource` base.
- Selection-store ownership and provider wiring remain the same; only the active-binding notification surface grew.
- The new interface is used by deferred startup hierarchy loading.
- Active hierarchy consumers that need a coherent "toolbar index + hierarchy viewmodel + note-list model" tuple can now
  subscribe to `activeBindingsChanged()`. The implementation emits that composite signal after selection changes and
  after provider mapping refreshes, so QML shells can refresh their entire active-binding snapshot in one step and
  avoid transient cross-domain list ghosts during hierarchy switches.
