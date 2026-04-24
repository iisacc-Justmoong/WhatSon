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
- `MobileHierarchyNavigationCoordinator` now also owns the canonical dismiss mapping for mobile back navigation, so the
  page shell can dismiss `detail -> editor -> note-list -> hierarchy` without depending on a fragile live router-pop
  stack.

## Verification Notes
- `test/cpp/suites/mobile_chrome_tests.cpp` exercises the route-state store normalization path and the sidebar-binding
  snapshot fallback path so mobile navigation helpers stay covered by the in-repo regression gate.
- The same suite now also pins `dismissPagePlan(...)` plus the source-locked `dismissCurrentPage()` wiring in
  `MobileHierarchyPage.qml`, so mobile back navigation cannot regress to raw `router.back()` pops from the editor.
