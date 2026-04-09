# `src/app/viewmodel/detailPanel/DetailPanelViewModel.cpp`

## Responsibility
`DetailPanelViewModel.cpp` owns the active detail-panel state, the active section view-model pointer, the toolbar
selection list, the dedicated `fileStat` statistics viewmodel, and the three `.wsnhead`-backed selector-copy objects
used by the properties form.

## Active State Surface
The exported `activeStateName()` now follows the corrected page ids:
- `properties`
- `fileStat`
- `insert`
- `layer`
- `fileHistory`
- `help`

## Notes
- Dedicated section view-model objects now use the same canonical naming as the exported page ids.
- The public string contract and the internal member/accessor names match; there is no alias layer between C++ and QML.
- The `fileStat` page no longer resolves to a generic placeholder section object; it now exposes
  `DetailFileStatViewModel` backed by the active header snapshot.
- The properties form selectors are now backed by dedicated `DetailHierarchySelectionViewModel` objects, so hierarchy clicks in the sidebar do not rewrite detail-panel combo state.
- `main.cpp` injects the canonical Projects/Bookmarks/Progress hierarchy viewmodels only as option sources for these copies.
- The current note context now follows `SidebarHierarchyViewModel` active bindings instead of being pinned to the library hierarchy, so the detail panel reads and writes the `.wsnhead` file for the note that the current workspace view actually identified.
- The current-note bridge now preserves the last valid note id and note directory path when the active sidebar domain does not expose a note list or a `noteDirectoryPathForNoteId(...)` contract, so the selector copies do not collapse back to `No ...` while the same note remains open in the workspace.
- A `currentNoteIdChanged` transition now triggers `reloadCurrentHeader(...)` immediately, not only `currentNoteDirectoryPathChanged`. This prevents folder/tag/project metadata from sticking to the previous note when the active note id changes but the resolved note-directory path string is temporarily unchanged.
- The current note-list model is now also observed for an optional `itemsChanged()` signal; when the active note stays selected and metadata changes elsewhere, the detail panel force-reloads the same `.wsnhead` file instead of waiting for a note-id transition.
- `reloadCurrentHeader(...)` now applies the loaded header snapshot to both `DetailPropertiesViewModel` and
  `DetailFileStatViewModel`, and clears both surfaces together when the note context is invalid.
- `writeProjectSelection(...)`, `writeBookmarkSelection(...)`, and `writeProgressSelection(...)` persist directly into the active note header file and then re-synchronize the selector copies from the file-backed session store.
- The shared write path now short-circuits when the incoming selector index already matches the detail-local selector-copy `selectedIndex`, so repeated `No ...` or same-option clicks do not re-persist identical `.wsnhead` state.
- After any detail-panel metadata write succeeds, the viewmodel now asks the active hierarchy domain,
  the injected project/bookmark/progress option-source domains, and the canonical Tags hierarchy
  viewmodel to `reloadNoteMetadataForNoteId(...)` so every affected note-list projection
  immediately mirrors the same `.wsnhead` state instead of waiting for a later hub reload.
- Each selector model now prepends a synthetic clear entry (`No project`, `No bookmark`, `No progress`), and selecting that entry clears the corresponding field in the current `.wsnhead` file instead of writing the visible label text.
- `assignFolderByName(...)` first resolves or creates the folder entry in `Folders.wsfolders`, then persists the resulting folder path/uuid binding into the active note header file, and finally re-applies the current header to the properties content view-model.
- `assignTagByName(...)` persists the chosen tag into the active note header file through the shared session store,
  re-applies the refreshed header to the properties content view-model, and re-selects the newly assigned tag row when
  the tag already existed on the note.
- `removeActiveFolder()` and `removeActiveTag()` delete the active metadata entry from the current note header file through the same file-backed session store and then re-apply the updated header to the properties content view-model.
- When a Progress hierarchy source is injected, the selector keeps that canonical source hierarchy
  intact instead of replacing the option list with the current note header's
  `progress enums="{...}"` order.
- The current note header's progress enums are now only a fallback option source when no canonical
  Progress hierarchy viewmodel is injected.
- Progress writes still resolve the stored integer from source metadata such as `itemId`,
  `progressValue`, and `progress:*`, so the combo returns the owning Progress hierarchy's numeric
  value instead of a note-local row index.
- Progress persistence resolves the enum integer from hierarchy entry metadata such as `itemId` or a numeric `progress:*` key; it is no longer hard-coded to the default `Ready/Pending/InProgress/Done` labels.
- Clearing progress writes the `.wsnhead` field as an explicit empty progress value, which round-trips back into the detail selector as `No progress`.
- The properties form now renders folder rows as leaf names extracted from `header.folders()` (for example `Archive/Knowledge` -> `Knowledge`) while preserving persisted `.wsnhead` full paths for write operations.
- Folder/tag add popups do not mutate the hierarchy source viewmodels directly; they only read those hierarchy nodes as
  option sources and keep the actual note-header write ownership inside `DetailPanelViewModel`.
