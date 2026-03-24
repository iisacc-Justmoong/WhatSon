# `src/app/qml/view/panels/sidebar/SidebarHierarchyRenameController.qml`

## Responsibility

This helper owns the sidebar inline-rename transaction. It decides whether the selected hierarchy row can be renamed,
seeds the temporary label, commits the bridge call, and unwinds focus/cancellation.

## Rename Target Resolution

- `beginRenameSelectedHierarchyItem()` resolves the selected hierarchy index from `hostView.selectedFolderIndex`.
- Before showing the editor, it calls `hostView.syncSelectedHierarchyItem(false)` and
  `hostView.refreshEditingHierarchyPresentation(true)` so the overlay is anchored to the selected LVRS row instead of
  a stale topmost generated item.
- The `Qt.callLater(...)` pass refreshes the presentation snapshot again after LVRS finishes any row regeneration.

## Commit / Cancel Rules

- `commitHierarchyRename()` delegates the actual rename to `hierarchyInteractionBridge.renameItem(...)`.
- Both commit and cancel clear the cached row presentation through `hostView.clearEditingHierarchyPresentation()` before
  resynchronizing LVRS selection.

## Label Handling

- `leafHierarchyItemLabel(...)` strips hierarchy path prefixes and keeps only the terminal display segment.
- `projectedHierarchyModel(...)` temporarily blanks the edited row label so the underlying text does not bleed through
  the inline input overlay.
