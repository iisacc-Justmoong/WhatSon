# `src/app/models/sidebar/SidebarHierarchyInteractionController.cpp`

## Implementation Notes
- Footer actions are normalized from either LVRS button index or explicit `eventName`.
- The controller keeps a one-turn queued footer action so LVRS versions that emit both config callbacks and
  `buttonClicked(...)` do not double-dispatch create, delete, or options actions.
- Expansion keys are scoped as `hierarchy:<activeIndex>:<stableKey>`, preserving row state across model refreshes without
  leaking identical row ids between hierarchy domains.
- Single-row and bulk expansion requests go through the bound `HierarchyInteractionBridge` by meta-object call, then
  roll back the preserved state if the bridge rejects the mutation.
- `setHierarchyInteractionBridge(...)` verifies the allowed View -> Controller edge but does not apply the startup
  mutable-wiring lock. This controller is QML-created together with `SidebarHierarchyView`, so applying the lock here
  would leave the chevron expansion path without a bound bridge after `main.cpp` freezes the root object graph.
- A first `onListItemExpanded` callback for a stable key is treated as a real expansion request and committed
  immediately, even when the separate pointer arm did not run. This covers the LVRS chevron `MouseArea` path where the
  row owns the click before the sidebar-level fallback sees a tap.
- Expansion commits invalidate pending activation attempts and start a short suppression timer, matching the previous
  chevron-click behavior while keeping the policy in C++.
