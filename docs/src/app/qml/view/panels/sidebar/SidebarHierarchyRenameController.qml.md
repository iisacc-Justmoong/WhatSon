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
- The post-create path uses `beginRenameHierarchyItemWhenVisible(...)`: it captures the controller's new selected index
  after `createFolder()`, refreshes the displayed model, activates the row by stable key when possible, and retries on
  later QML turns until the row has non-zero geometry. Inline rename state is not entered while the row is missing, so
  the input field cannot appear over `All Library` or another stale active row.
- The row-readiness guard also checks the visual `HierarchyItem` identity against the selected model row's stable key.
  This prevents a newly inserted blank folder row from hiding its label while the input field is still positioned over a
  stale system-bucket row.
- The post-create path keeps selection activation separate from focus. `syncSelectedHierarchyItem(...)` may still align
  the active LVRS row, but inline rename focus is scheduled through `scheduleHierarchyRenameFieldFocus(...)` and applied
  directly to the `LV.InputField`. The repeated deferred passes absorb hierarchy row regeneration after creation without
  sending active focus back to the sidebar root.
- New-folder rename uses `beginRenameHierarchyItemKeyWhenVisible(...)` when a stable inserted key is available. The key
  is derived from the before/after hierarchy model diff in the host view, so a stale or briefly reset
  `hierarchySelectedIndex` cannot place the editor over a system bucket while the newly inserted folder row is elsewhere.

## Commit / Cancel Rules

- `commitHierarchyRename()` delegates the actual rename to `hierarchyInteractionBridge.renameItem(...)`.
- Both commit and cancel clear the cached row presentation through `hostView.clearEditingHierarchyPresentation()` before
  resynchronizing LVRS selection.
- Both commit and cancel also force `hostView.syncDisplayedHierarchyModel(true)` after the rename state is cleared, so
  the rendered hierarchy row reflects the final controller state immediately after the transaction ends.

## Label Handling

- `leafHierarchyItemLabel(...)` now resolves the edit label from the hierarchy item's escaped `id/path` first instead
  of naively splitting the rendered label on `/`.
- Literal-slash folder names such as `Marketing/Sales` therefore stay one rename target when the persisted hierarchy
  path is `Marketing\\/Sales`; the inline editor no longer collapses them to only `Sales`.
- Starting inline rename does not rebuild `displayedHierarchyModel` just to hide the edited label. The input overlay
  uses the captured row presentation directly, which keeps newly created folder geometry stable while focus is applied.

## Tests

- The maintained C++ regression suite now also pins the escape-aware rename-label path for literal-slash folder names.
- Regression checklist for this helper:
  - Pressing `Enter` on a renamable folder must open the inline input without replacing the visible row label with a
    UUID-like fallback value.
  - A folder label such as `Marketing/Sales` must seed the inline editor with the full literal label, not only the
    terminal `Sales` segment.
  - Committing a rename must restore the rendered row label immediately after the overlay closes.
  - Cancelling a rename must likewise restore the original rendered row label immediately.
