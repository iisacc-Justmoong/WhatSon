# `src/app/qml/view/panels/detail/DetailContents.qml`

## Responsibility
`DetailContents.qml` owns the state-switched body of the desktop detail panel.
The implemented primary form is the Figma `Properties` example (`155:4582`), while the other toolbar states mount dedicated placeholder forms instead of reusing the properties body.

## Root Contract
- Root `objectName`: `DetailContents`
- Root Figma node id: `155:4582`
- Inputs:
  - `activeContentViewModel`
  - `activeStateName`
  - `detailPanelViewModel`
  - `projectSelectionViewModel`
  - `bookmarkSelectionViewModel`
  - `progressSelectionViewModel`
- Normalized state values:
  - `properties`
  - `fileStat`
  - `insert`
  - `fileHistory`
  - `layer`
  - `help`
- Legacy aliases are intentionally not accepted. Callers must send the canonical state ids above.

## Implemented Properties Form
The `properties` state renders the Figma `Form` node (`155:4583`) with the exact section order:
1. `Projects` combo (`178:5494`, text `178:5495`, combo `178:5496`)
2. `Bookmark` combo (`155:4584`, text `155:4585`, combo `155:4586`)
3. `FoldersList` (`155:4587`, text `155:4588`, list `155:4589`)
4. `TagsList` (`155:4590`, text `155:4591`, list `155:4592`)
5. `Progress` combo (`178:5501`, text `178:5502`, combo `178:5503`)

## Selector Wiring
- The `Projects`, `Bookmark`, and `Progress` selectors do not own ad-hoc option lists.
- Each selector receives a dedicated detail-panel-local selector viewmodel from `DetailPanel.qml`.
- Those selector viewmodels mirror hierarchy entries from the canonical Projects/Bookmarks/Progress hierarchy viewmodels, but they keep their own local `selectedIndex`.
- Each selector builds its popup entries from the matched detail-local selector viewmodel's `hierarchyModel`.
- For `Progress`, the popup keeps the injected canonical Progress hierarchy options when that source
  exists; it falls back to the current `.wsnhead` enum labels only when no Progress source model is
  available.
- Each selector popup starts with a synthetic clear item:
  - `Projects`: `No project`
  - `Bookmark`: `No bookmark`
  - `Progress`: `No progress`
- Each popup item forwards `iconName` and `iconSource` from the hierarchy entry when those fields are present, so the detail-panel menu mirrors the hierarchy icon contract instead of falling back to LVRS defaults.
- Each visible combo label mirrors the matched selector-copy `selectedIndex` through `itemLabel(...)`.
- Selecting a synthetic `No ...` popup entry clears the corresponding `.wsnhead` field instead of writing the visible label text.
- Selecting a popup entry writes the chosen index back through that selector-copy object only.
- Re-selecting the already active popup index is treated as a no-op and does not request another detail write, preventing recursive `reloadNoteMetadataForNoteId(...)` loops from context re-entry.
- This keeps the detail panel aligned with the canonical hierarchy item lists without sharing mutable selection state with the sidebar.

## LVRS Reuse
- Uses `LV.ComboBox` for all compact selectors.
- Uses `LV.ContextMenu` to present all three selector popups from shared hierarchy view-model data.
- Uses `LV.HierarchyItem` for the compact `Folders` and `Tags` rows so each metadata entry can hold an explicit active selection state inside the small-list surface.
- Uses `LV.ListFooter` for the `addFile` / `trash` / `settings` footer controls in the Figma small lists.
- The `trash` control keeps the Figma logical name but pins its rendered icon source to `generaldelete`, because that is the shipped trash-can asset in LVRS.
- Uses LVRS typography and panel tokens instead of introducing ad-hoc colors or fonts.

## Label Tokens
The `Projects`, `Bookmark`, and `Progress` combo labels use the caption text token color (`LV.Theme.captionColor`).

## Metadata List Interaction
- `FoldersList` and `TagsList` derive their active row index from the properties content view-model instead of storing ad-hoc QML-only state.
- `FoldersList` mirrors each `.wsnhead` folder path verbatim, so `Research/Ideas` remains `Research/Ideas` in the visible list instead of collapsing to the final segment.
- Clicking a metadata row updates the corresponding `activeFolderIndex` or `activeTagIndex` property on the active properties view-model.
- `FoldersList` now uses the footer `addFile` control to open an inline `LV.InputField` row inside the list viewport.
- The inline folder editor is a temporary blank list item overlay, not a detached popup, so the add flow stays inside the Figma small-list surface.
- The inline folder editor accepts raw folder text and forwards successful confirmation to `detailPanelViewModel.assignFolderByName(...)`.
- While the inline folder editor is open, the folder footer `add`, `trash`, and `settings` actions are disabled to keep the temporary row state single-owner.
- The footer `trash` action is disabled until a row is active.
- When the footer `trash` action is triggered, `DetailContents.qml` calls `detailPanelViewModel.removeActiveFolder()` or `detailPanelViewModel.removeActiveTag()` directly so the selected `.wsnhead` metadata entry is removed from the file-backed session store instead of only emitting a view hook.
- Folder add confirmation writes through the detail-panel view-model instead of mutating the list locally, so the displayed list remains a direct mirror of persisted file state.
- The visible `FoldersList` and `TagsList` must also refresh when the active note stays the same but its note-list model emits `itemsChanged()`, because that signal indicates the current `.wsnhead` metadata may have changed outside the detail-panel write path.

## Non-Properties States
For `fileStat`, `insert`, `fileHistory`, `layer`, and `help`, the file renders a distinct placeholder form with state-specific titles and summaries.
This keeps the state switch explicit until each mode receives its final Figma form.
