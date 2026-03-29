# `src/app/viewmodel/panel/HierarchyInteractionBridge.hpp`

## Role
This header defines the bridge used by QML hierarchy views for non-drag interaction commands.

It adapts a generic `QObject*` hierarchy viewmodel input into a capability-aware surface that answers questions such as:
- can this row be renamed?
- may this hierarchy create a folder?
- may the selected folder be deleted?
- may a row expand or collapse?

## Why It Accepts `QObject*`
QML can pass object references easily, but it does not know about C++ interface inheritance. The bridge therefore accepts a raw `QObject*`, casts it to `IHierarchyViewModel`, and then probes for capability interfaces at runtime.

## Public Surface
- Properties:
  - `hierarchyViewModel`
  - `renameContractAvailable`
  - `createFolderEnabled`
  - `deleteFolderEnabled`
  - `viewOptionsEnabled`
- Invokables:
  - `canRenameItem(...)`
  - `renameItem(...)`
  - `createFolder()`
  - `deleteSelectedFolder()`
  - `setItemExpanded(...)`
  - `setAllItemsExpanded(...)`

## Expected Consumer
`SidebarHierarchyView.qml` is the primary consumer of this bridge.
