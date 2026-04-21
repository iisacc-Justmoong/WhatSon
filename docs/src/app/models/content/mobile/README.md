# `src/app/models/content/mobile`

## Responsibility
`src/app/models/content/mobile` contains QObject-based coordinators and state stores used by the mobile workspace flow.

## Scope
- Mirrored source directory: `src/app/models/content/mobile`
- Child directories: 0

## Key Helpers
- `MobileHierarchyRouteStateStore` preserves route-adjacent selection state such as the last observed route path, the
  note-list selection to restore, and pop-repair request ids.
- `MobileHierarchySelectionCoordinator` snapshots sidebar hierarchy bindings and resolves the currently selected
  hierarchy index from active mobile content viewmodels.
- `MobileHierarchyCanonicalRoutePlanner`, `MobileHierarchyNavigationCoordinator`, and
  `MobileHierarchyBackSwipeCoordinator` keep route transitions deterministic while the mobile scaffold swaps between
  hierarchy, editor, and detail surfaces.

## Verification Notes
- `test/cpp/suites/mobile_chrome_tests.cpp` exercises the route-state store normalization path and the sidebar-binding
  snapshot fallback path so mobile navigation helpers stay covered by the in-repo regression gate.
