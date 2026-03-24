# `src/app/viewmodel/hierarchy/IHierarchyCapabilities.hpp`

## Role
This header contains the write-side capability taxonomy for hierarchy viewmodels.

Instead of forcing every hierarchy implementation to expose every possible mutation, the repository now uses small interfaces:
- `IHierarchyRenameCapability`
- `IHierarchyCrudCapability`
- `IHierarchyExpansionCapability`
- `IHierarchyReorderCapability`
- `IHierarchyNoteDropCapability`

## Why This Matters
Different hierarchy domains have different mutation rules.
- Some can be renamed but not reordered.
- Some can create folders but not accept note drops.
- Some are display-only for parts of the UI.

The capability split prevents generic consumers from assuming functionality that a domain does not actually support.

## Design Notes
- Every capability is declared as a Qt interface through `Q_DECLARE_INTERFACE`.
- QML-facing bridge objects use `qobject_cast<CapabilityType*>` at runtime.
- Capability availability is therefore both type-safe and dynamic.

## Typical Consumers
- `HierarchyInteractionBridge` uses rename, CRUD, and expansion capabilities.
- `HierarchyDragDropBridge` uses reorder and note-drop capabilities.
- Domain viewmodels implement only the interfaces they need.
