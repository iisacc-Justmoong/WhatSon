# `src/app/viewmodel/sidebar/HierarchyViewModelProvider.hpp`

## Role
`HierarchyViewModelProvider` resolves a hierarchy-domain index to the dedicated domain viewmodel and note-list model.

## Current Shape
- Uses `Mapping { hierarchyIndex, viewModel }` entries instead of one hard-coded struct field per hierarchy domain.
- Stores mappings in an index-normalized registry keyed by `HierarchySidebarDomain.hpp`.
- Keeps the resolution path open for extension without requiring a new member variable or switch branch for every new
  hierarchy domain.

## Boundary
- The composition root still owns concrete registration.
- Consumers below that line only depend on `IHierarchyViewModelProvider`.
