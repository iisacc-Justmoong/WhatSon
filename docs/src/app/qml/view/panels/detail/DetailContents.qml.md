# `src/app/qml/view/panels/detail/DetailContents.qml`

## Responsibility
`DetailContents.qml` owns the state-switched body of the desktop detail panel.
The implemented primary forms are the Figma `Properties` example (`155:4582`) and the live `fileStat` statistics form.
Only the remaining toolbar states still mount dedicated placeholders instead of reusing the properties body.

## Root Contract
- Root `objectName`: `DetailContents`
- Root Figma node id: `155:4582`
- Inputs:
  - `activeContentController`
  - `activeStateName`
  - `noteContextLinked`
  - `detailPanelController`
  - `fileStatController`
  - `projectSelectionController`
  - `bookmarkSelectionController`
  - `progressSelectionController`
- Normalized state values:
  - `detached`
  - `properties`
  - `fileStat`
  - `insert`
  - `fileHistory`
  - `layer`
  - `help`
- Legacy aliases are intentionally not accepted. Callers must send the canonical state ids above.
- `detached` is resolved when the panel is not linked to a valid note context; in this state every form surface stays hidden.

## Implemented Properties Form
The `properties` state renders the Figma `Form` node (`155:4583`) with the exact section order:
1. `Projects` combo (`178:5494`, text `178:5495`, combo `178:5496`)
2. `Bookmark` combo (`155:4584`, text `155:4585`, combo `155:4586`)
3. `FoldersList` (`155:4587`, text `155:4588`, list `155:4589`)
4. `TagsList` (`155:4590`, text `155:4591`, list `155:4592`)
5. `Progress` combo (`178:5501`, text `178:5502`, combo `178:5503`)

## Selector Wiring
- The `Projects`, `Bookmark`, and `Progress` selectors do not own ad-hoc option lists.
- Each selector receives a dedicated detail-panel-local selector controller from `DetailPanel.qml`.
- Those selector controllers mirror hierarchy entries from the canonical Projects/Bookmarks/Progress hierarchy controllers, but they keep their own local `selectedIndex`.
- Each selector builds its popup entries from the matched detail-local selector controller's `hierarchyModel`.
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
- Uses the shared `DetailMetadataHierarchyPicker.qml` overlay to render existing Library/Tags hierarchy entries for
  metadata-add actions; the picker is an overridden `LV.ContextMenu`, so the desktop popup and mobile sheet both keep
  LVRS menu animation/dismissal behavior while rendering a flat `LV.HierarchyItem` list inside the menu body.
- The `trash` control keeps the Figma logical name but pins its rendered icon source to `generaldelete`, because that is the shipped trash-can asset in LVRS.
- Uses LVRS typography and panel tokens instead of introducing ad-hoc colors or fonts.
- Compact detail-list and footer controls use `LV.Theme.gapNone` and `LV.Theme.accentTransparent` for zero spacing
  and transparent button backgrounds instead of raw visual literals.

## Scale-Aware Layout
- The shared properties form now derives section heights from their child layout instead of keeping fixed `33px` combo
  or `155px` list-section clamps.
- Compact rows, list cards, footer bars, and form insets use named `LV.Theme` token compositions so LVRS mobile UI
  scaling increases both control size and the surrounding gaps together.
- The properties form must remain readable under the mobile `1.5x` LVRS scale; section titles, combo boxes, list
  surfaces, and list footers must not overlap each other.

## Label Tokens
The `Projects`, `Bookmark`, and `Progress` combo labels use the caption text token color (`LV.Theme.captionColor`).

## Metadata List Interaction
- `FoldersList` and `TagsList` derive their active row index from the properties content view-model instead of storing ad-hoc QML-only state.
- `FoldersList` mirrors each `.wsnhead` folder path verbatim, so `Research/Ideas` remains `Research/Ideas` in the visible list instead of collapsing to the final segment.
- Clicking a metadata row now routes through the inline `metadataSelectionController` owned by each `DetailListSection`.
- Selection behavior for both `FoldersList` and `TagsList`:
  - plain click: single selection only
  - `Cmd/Ctrl + click`: toggle selection
  - `Shift + click`: contiguous range selection
  - `Cmd/Ctrl + Shift + click`: union the range with the existing selection
- The controller binds `section: listSection`, keeps a QML-local visual `selectedIndices` set, and still commits one
  primary active row back through
  `activeFolderIndex` or `activeTagIndex` so existing delete and active-row behaviors continue to use a single index.
- The footer `addFile` control no longer writes metadata inline by default.
- Pressing `FoldersList` add opens the shared hierarchy picker anchored from the footer on desktop and as a bottom
  sheet on mobile.
- `DetailContents.qml` opens that picker through `openForAnchor(...)`, making the caller explicitly hand off footer
  anchoring to the `LV.ContextMenu`-derived component instead of treating it like a generic popup.
- `resolveMetadataPickerItems(...)` forces every hierarchy row to `expanded: true` and `showChevron: false`, so the
  add picker always opens with the full tree already unfolded and without collapse controls.
- The folder picker filters out Library system buckets such as `All`, `Draft`, and `Today`, leaving only assignable
  folder hierarchy nodes.
- Choosing a folder entry forwards the canonical folder path (`entry.id`) into `detailPanelController.assignFolderByName(...)`,
  so duplicate leaf labels still resolve to the correct full hierarchy path.
- The folder picker also exposes a single fallback action, `Create custom folder path`, which re-opens the existing
  inline folder editor for first-folder or ad-hoc folder creation.
- Pressing `TagsList` add now opens the same picker shell against the canonical Tags hierarchy instead of emitting only
  a view hook with no usable add surface.
- Choosing a tag entry forwards the canonical tag id/label into `detailPanelController.assignTagByName(...)`, so the
  user can add note tags without typing when the tag already exists in the hub hierarchy.
- While the inline folder editor is open, the folder footer `add`, `trash`, and `settings` actions are disabled to keep the temporary row state single-owner.
- The footer `trash` action is disabled until a row is active.
- When the footer `trash` action is triggered, `DetailContents.qml` calls `detailPanelController.removeActiveFolder()` or `detailPanelController.removeActiveTag()` directly so the selected `.wsnhead` metadata entry is removed from the file-backed session store instead of only emitting a view hook.
- Folder add confirmation writes through the detail-panel view-model instead of mutating the list locally, so the displayed list remains a direct mirror of persisted file state.
- The visible `FoldersList` and `TagsList` must also refresh when the active note stays the same but its note-list model emits `itemsChanged()`, because that signal indicates the current `.wsnhead` metadata may have changed outside the detail-panel write path.
- The compact metadata rows disable `LV.HierarchyItem`'s standalone activation and rely on the explicit selection
  controller instead, preventing stale per-row activation state from making the list look permanently multi-selected.
- Because those rows are non-activatable (`activatable: false`), the row's inactive fill is now explicitly bound to
  `rowSelected`:
  - unselected row: transparent background
  - selected row: `LV.Theme.accentBlueMuted`
  so `FoldersList` / `TagsList` no longer look selected when no row is selected.

## File Statistics State
- The `fileStat` state now mounts `DetailFileStatForm.qml`.
- That form receives the dedicated `fileStatController` input when the resolved state is `fileStat`, so its text rows
  are driven by the explicit `DetailFileStatController` contract instead of a generic active-state object.
- The statistics surface is no longer a placeholder; it renders the persisted `.wsnhead <fileStat>` values and the
  current header metadata as three `description`-styled text blocks matching the Figma node `235:7734`.

## Detached State
- The root `state` now resolves from `noteContextLinked` first:
  - linked note context -> canonical active state (`properties`, `fileStat`, ...)
  - unlinked note context -> `detached`
- `detached` closes metadata pickers/inline folder editing and renders no form content, preventing stale note metadata
  from remaining visible after the note link is lost.

## Remaining Placeholder States
For `insert`, `fileHistory`, `layer`, and `help`, the file still renders a distinct placeholder form with state-specific titles and summaries.
This keeps the state switch explicit until each mode receives its final Figma form.

## Tests

- `qmlInlineSelectionHelpers_bindOwnersAfterControllerFileDeletion` locks the inline metadata helper contract:
  `metadataSelectionController` must bind `section: listSection` and must not retain stale `controller.*` references.
- Regression checklist:
  - Plain click in `FoldersList` must clear any prior multi-selection and leave only the clicked row selected.
  - `Cmd/Ctrl + click` and `Shift + click` must keep modifier-based multi-selection working in `FoldersList`.
  - The same selection rules must also hold for `TagsList`, because both lists share the same `DetailListSection` and
    selection controller.
  - The inline metadata selection controller must keep `section: listSection` and avoid unresolved `controller.*`
    self-references, otherwise the detail panel logs runtime `ReferenceError` during workspace startup.
  - Desktop `FoldersList` add must open an anchored hierarchy popup instead of jumping straight into inline text entry.
  - Mobile `FoldersList` and `TagsList` add must open the bottom-sheet picker through the same overridden `LV.ContextMenu`.
  - The folder and tag pickers must show every hierarchy row immediately, without chevrons or collapsed descendants.
  - Choosing an existing Library folder or Tags entry from the picker must persist into the active `.wsnhead` file and
    then refresh the detail-panel list contents without changing the current note selection.
  - The folder picker must never offer Library system buckets such as `All`, `Draft`, or `Today` as assignable
    targets.
  - With LVRS mobile UI scaling enabled, the `Projects`, `Bookmark`, `Folders`, `Tags`, and `Progress` sections must
    keep their vertical order without label/control or footer/next-section overlap.
  - When `activeFolderIndex`/`activeTagIndex` is `-1`, every metadata row background in `FoldersList` and `TagsList`
    must stay transparent (no pseudo-selected fill).
  - When no note is selected (or the selected note cannot resolve/load `.wsnhead`), `DetailContents` must be in
    `detached` and no properties/file-stat/placeholder form content should be visible.
