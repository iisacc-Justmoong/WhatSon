# `src/app/policy/ArchitecturePolicyLock.cpp`

## Role
This file contains the concrete architectural contract matrix and the one-way runtime lock used by the application startup path.

## Dependency Matrix
The matrix is hard-coded and intentionally simple.
- `View` may depend on `Controller`.
- `Controller` may depend on `DataModel`, `Store`, `Parser`, and `Creator`.
- `Store` may depend on `DataModel`, `Parser`, `Creator`, and `FileSystem`.
- `Parser` and `Creator` may depend on `DataModel`.
- `DataModel` and `FileSystem` do not gain outgoing dependencies through this policy.

This is not a general-purpose DI framework. It is a repository-specific guardrail.

## Runtime Lock
`g_architecturePolicyLocked` is a process-wide atomic flag.
- It starts unlocked.
- Startup code performs dependency injection.
- `ArchitecturePolicyLock::lock()` flips the flag.
- Late wiring attempts can then reject mutation and emit warnings.
- `ArchitecturePolicyLock::unlockForTests()` exists only so regression tests can reset the global lock state between cases.

Production composition still treats the lock as one-way: application wiring is expected to be complete before the first QML scene is fully active.

## Verification Helpers
- `assertDependencyAllowed(...)` is the pure diagnostic helper.
- `verifyDependencyAllowed(...)` adds a production-facing warning path using `[whatson:policy][dependency] ...`.
- `verifyMutableWiringAllowed(...)` adds the post-lock rewiring guard using `[whatson:policy][lock] ...`.
- `verifyMutableDependencyAllowed(...)` is the shared helper now used by setter/wiring entry points that must reject both illegal layer edges and any mutation attempted after the startup lock.

The newer bridge and Controller setter code uses these shared helpers directly, so the policy is exercised in real wiring paths instead of remaining documentation-only.

## Practical Reading
Read this file together with:
- `src/app/main.cpp` for lock timing.
- `src/app/models/panel/HierarchyInteractionBridge.cpp` for view-to-controller verification.
- `src/app/models/panel/HierarchyDragDropBridge.cpp` for drag/drop contract verification.
- `src/app/models/sidebar/SidebarHierarchyController.cpp` for controller-to-store verification.
- `src/app/models/sidebar/HierarchyControllerProvider.cpp`, `src/app/models/panel/NoteListModelContractBridge.cpp`, and `src/app/models/detailPanel/DetailCurrentNoteContextBridge.cpp` for post-lock mutation rejection at major runtime wiring seams.
