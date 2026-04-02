# `src/app/viewmodel/hierarchy/WhatSonHierarchyTreeItemSupport.hpp`

## Responsibility

This header centralizes the repeated tree-item mutation helpers used by hierarchy support modules.

- recompute `showChevron` from depth relationships
- generate the next `FolderN` sequence
- rename an item label with shared validation
- detect bucket-header rows
- create flat or nested folder rows
- delete a selected subtree and normalize the next selection index

## Public Helpers

- `applyChevronByDepth(...)`
- `nextGeneratedFolderSequence(...)`
- `renameHierarchyItem(...)`
- `isBucketHeaderItem(...)`
- `createFlatHierarchyFolder(...)`
- `createNestedHierarchyFolder(...)`
- `deleteHierarchySubtree(...)`

## Variants

Two insertion strategies are intentionally separated:

- `createFlatHierarchyFolder(...)`: append or insert a root-level folder after the selected row
- `createNestedHierarchyFolder(...)`: insert a child folder after the selected subtree, with optional parent expansion

This keeps the domain wrappers small while removing the duplicated mutation code from each support header.

## Consumers

This helper is re-exported by:

- `ProjectsHierarchyViewModelSupport.hpp`
- `EventHierarchyViewModelSupport.hpp`
- `BookmarksHierarchyViewModelSupport.hpp`
- `ProgressHierarchyViewModelSupport.hpp`
- `PresetHierarchyViewModelSupport.hpp`
