# `src/app/viewmodel/hierarchy/tags/TagsHierarchyViewModel.hpp`

## Responsibility

This header defines the dedicated MVVM boundary for the tags hierarchy. The viewmodel owns the
UI-facing row model, the tag-projected note list, selection state, CRUD entry points, and LVRS
expansion hooks for the tag tree.

## Public Contract

- Exposes `itemModel`, `hierarchyModel`, `selectedIndex`, `itemCount`, and load-state properties to
  QML.
- Exposes `noteListModel` so the Tags sidebar domain can drive the shared list bar with tag-filtered
  notes instead of leaving the domain without a note projection.
- Implements `IHierarchyViewModel`, `IHierarchyRenameCapability`, `IHierarchyCrudCapability`, and
  `IHierarchyExpansionCapability`.
- Provides `setItemExpanded(int, bool)` so LVRS hierarchy rows can persist user-driven fold state
  back into the viewmodel instead of treating expansion as transient view-only state.
- Accepts both imperative `setDepthItems(...)` input and runtime loader input through
  `applyRuntimeSnapshot(...)`.
- Exposes `requestViewModelHook()` to trigger a file-backed `Tags.wstags` reload through the same
  snapshot-application path.
- Declares inherited capability methods with explicit `override` markers so compiler diagnostics
  catch interface drift immediately and builds stay warning-clean.

## State And Persistence Rules

- `m_entries` is the canonical parsed tag tree payload.
- `m_items` is the rendered LVRS row state, including `expanded`.
- `m_allNotes` is the indexed `.wshub` note cache used to project the currently selected tag subtree
  into `m_noteListModel`.
- `m_tagsFilePath` tracks the writable `Tags.wstags` target for rename/create/delete mutations.
- `m_createdFolderSequence` is derived from the current data so generated labels do not collide with
  existing tag folders after reload.

## Refresh Semantics

`applyRuntimeSnapshot(...)` is expected to behave conservatively.

- A load failure updates the error state without mutating the current visible hierarchy.
- An unchanged tag snapshot must not rebuild the hierarchy rows, but it still needs to re-index note
  metadata from the current `.wshub` so tag-to-note projection stays fresh.
- A changed snapshot may rebuild rows, but it must preserve both selection and expansion by stable
  tag keys so watcher-driven reloads do not collapse the tree.

## QML Integration

QML callers should treat `selectedIndex` and `setItemExpanded(...)` as the only supported mutable
view-state entry points. `noteDirectoryPathForNoteId(...)` and `reloadNoteMetadataForNoteId(...)`
also exist as cross-panel synchronization hooks so the detail panel can refresh the tag projection
after it mutates `.wsnhead` tags. Direct row reconstruction in QML would bypass the viewmodel's
persistence and refresh constraints.
