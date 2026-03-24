# `src/app/viewmodel/hierarchy/library/LibraryNoteMutationViewModel.cpp`

## Role
This implementation is a forwarding layer, not a second source of truth.

It keeps a `QPointer` to `LibraryHierarchyViewModel`, forwards selected mutation calls, and re-emits a small subset of mutation-related signals.

## Implementation Pattern
- On `setSourceViewModel(...)`, the object disconnects the previous source, stores the new one, and connects to the source's destruction and mutation signals.
- The mutation functions simply return `false` if the source is absent.
- `handleSourceDestroyed()` clears all connections and emits `sourceViewModelChanged()`.

## Why This Is Useful
The value of this file is architectural, not algorithmic.
- It narrows the writable note mutation surface.
- It reduces the number of consumers that need to depend on `LibraryHierarchyViewModel`.
- It creates a stable seam for future extraction of mutation logic away from the library hierarchy viewmodel.
