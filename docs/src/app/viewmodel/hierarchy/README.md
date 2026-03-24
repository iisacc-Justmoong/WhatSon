# `src/app/viewmodel/hierarchy`

## Role
This directory contains the shared hierarchy contract and all domain-specific hierarchy implementations.

The key architectural rule is that each hierarchy category owns its own runtime viewmodel instance. There is no shared flat hierarchy viewmodel across domains.

## Current Contract Shape
- `IHierarchyViewModel` is now the read-oriented base contract consumed broadly by QML and panel bridges.
- `IHierarchyCapabilities` defines narrow write-oriented capabilities such as rename, CRUD, expansion, reorder, and note-drop support.
- Domain directories such as `library`, `projects`, `bookmarks`, `tags`, and others implement only the capabilities they actually support.

## Why This Split Exists
Older hierarchy work tended to accumulate all behavior onto a single interface, which pushed unrelated mutation responsibilities into places that only needed read access. The capability split keeps the common surface smaller and makes write paths explicit.

## Reading Order
1. `IHierarchyViewModel.hpp`
2. `IHierarchyCapabilities.hpp`
3. A domain-specific viewmodel header such as `library/LibraryHierarchyViewModel.hpp`
4. Any panel bridge that consumes the contract
