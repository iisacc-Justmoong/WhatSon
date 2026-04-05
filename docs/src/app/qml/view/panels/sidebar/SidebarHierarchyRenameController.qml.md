# `src/app/qml/view/panels/sidebar/SidebarHierarchyRenameController.qml`

## Responsibility

This helper owns the sidebar inline-rename transaction. It decides whether the selected hierarchy row can be renamed,
seeds the temporary label, commits the bridge call, and unwinds focus/cancellation.

## Rename Target Resolution

- `beginRenameSelectedHierarchyItem()` resolves the selected hierarchy index from `hostView.selectedFolderIndex`.
- Before showing the editor, it calls `hostView.syncSelectedHierarchyItem(false)` and
  `hostView.refreshEditingHierarchyPresentation(true)` so the overlay is anchored to the selected LVRS row instead of
  a stale topmost generated item.
- The same entry path now also forces `hostView.syncDisplayedHierarchyModel(true)` so the temporary projection for the
  edited row is applied immediately, instead of waiting for an unrelated hierarchy refresh.
- The `Qt.callLater(...)` pass refreshes the presentation snapshot again after LVRS finishes any row regeneration.

## Commit / Cancel Rules

- `commitHierarchyRename()` delegates the actual rename to `hierarchyInteractionBridge.renameItem(...)`.
- Both commit and cancel clear the cached row presentation through `hostView.clearEditingHierarchyPresentation()` before
  resynchronizing LVRS selection.
- Both commit and cancel also force `hostView.syncDisplayedHierarchyModel(true)` after the rename state is cleared, so a
  blanked edit-row projection cannot survive after the transaction ends.

## Label Handling

- `leafHierarchyItemLabel(...)` strips hierarchy path prefixes and keeps only the terminal display segment.
- `projectedHierarchyModel(...)` temporarily blanks the edited row label so the underlying text does not bleed through
  the inline input overlay.
- The projection must preserve stable row identity fields such as `key` and `itemKey`; blanking those fields allows
  LVRS fallback identifiers to leak into the visible row after subsequent interactions.

## Tests

- Automated test files are not currently present in this repository.
- Regression checklist for this helper:
  - Pressing `Enter` on a renamable folder must open the inline input without replacing the visible row label with a
    UUID-like fallback value.
  - Committing a rename must restore the rendered row label immediately after the overlay closes.
  - Cancelling a rename must likewise restore the original rendered row label immediately.
