# `src/app/viewmodel/sidebar/SidebarHierarchyViewModel.cpp`

## Role
This implementation normalizes the active sidebar hierarchy index and resolves the current hierarchy and note-list models through injected collaborators.

## Core Behavior
- If a selection store exists, the store is the source of truth for the active hierarchy index.
- If no selection store exists yet, the class falls back to an internal index.
- `hierarchyViewModelForIndex(...)` and `noteListModelForIndex(...)` delegate to the provider after index normalization.

## Wiring Rules
- `setSelectionStore(...)` verifies the `ViewModel -> Store` edge through the policy layer.
- Both `setSelectionStore(...)` and `setViewModelProvider(...)` reject rewiring after `ArchitecturePolicyLock::lock()` has been called.
- When a new selection store is attached, the previously active index is preserved and copied into the store.

## Why This Matters
This file is the seam between generic sidebar UI and the dedicated hierarchy-per-domain runtime architecture. If sidebar switching is wrong, stale, or inconsistent, this is one of the first files to inspect.
