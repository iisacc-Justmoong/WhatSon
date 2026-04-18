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

- `leafHierarchyItemLabel(...)` now resolves the edit label from the hierarchy item's escaped `id/path` first instead
  of naively splitting the rendered label on `/`.
- Literal-slash folder names such as `Marketing/Sales` therefore stay one rename target when the persisted hierarchy
  path is `Marketing\\/Sales`; the inline editor no longer collapses them to only `Sales`.
- `projectedHierarchyModel(...)` temporarily blanks the edited row label so the underlying text does not bleed through
  the inline input overlay.
- The projection must preserve stable row identity fields such as `key` and `itemKey`; blanking those fields allows
  LVRS fallback identifiers to leak into the visible row after subsequent interactions.

## Tests

- The maintained C++ regression suite now also pins the escape-aware rename-label path for literal-slash folder names.
- Regression checklist for this helper:
  - Pressing `Enter` on a renamable folder must open the inline input without replacing the visible row label with a
    UUID-like fallback value.
  - A folder label such as `Marketing/Sales` must seed the inline editor with the full literal label, not only the
    terminal `Sales` segment.
  - Committing a rename must restore the rendered row label immediately after the overlay closes.
  - Cancelling a rename must likewise restore the original rendered row label immediately.
