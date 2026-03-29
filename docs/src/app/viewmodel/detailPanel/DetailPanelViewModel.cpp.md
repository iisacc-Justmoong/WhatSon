# `src/app/viewmodel/detailPanel/DetailPanelViewModel.cpp`

## Responsibility
`DetailPanelViewModel.cpp` owns the active detail-panel state, the active section view-model pointer, the toolbar selection list, and the three `.wsnhead`-backed selector-copy objects used by the properties form.

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
- The properties form selectors are now backed by dedicated `DetailHierarchySelectionViewModel` objects, so hierarchy clicks in the sidebar do not rewrite detail-panel combo state.
- `main.cpp` injects the canonical Projects/Bookmarks/Progress hierarchy viewmodels only as option sources for these copies.
- The current note context now follows `SidebarHierarchyViewModel` active bindings instead of being pinned to the library hierarchy, so the detail panel reads and writes the `.wsnhead` file for the note that the current workspace view actually identified.
- The current-note bridge now preserves the last valid note id and note directory path when the active sidebar domain does not expose a note list or a `noteDirectoryPathForNoteId(...)` contract, so the selector copies do not collapse back to `No ...` while the same note remains open in the workspace.
- The current note-list model is now also observed for an optional `itemsChanged()` signal; when the active note stays selected and metadata changes elsewhere, the detail panel force-reloads the same `.wsnhead` file instead of waiting for a note-id transition.
- `writeProjectSelection(...)`, `writeBookmarkSelection(...)`, and `writeProgressSelection(...)` persist directly into the active note header file and then re-synchronize the selector copies from the file-backed session store.
- After any detail-panel metadata write succeeds, the viewmodel now asks the active hierarchy domain to `reloadNoteMetadataForNoteId(...)` so the visible note list immediately mirrors the same `.wsnhead` state instead of waiting for a later hub reload.
- Each selector model now prepends a synthetic clear entry (`No project`, `No bookmark`, `No progress`), and selecting that entry clears the corresponding field in the current `.wsnhead` file instead of writing the visible label text.
- `assignFolderByName(...)` first resolves or creates the folder entry in `Folders.wsfolders`, then persists the resulting folder path/uuid binding into the active note header file, and finally re-applies the current header to the properties content view-model.
- `removeActiveFolder()` and `removeActiveTag()` delete the active metadata entry from the current note header file through the same file-backed session store and then re-apply the updated header to the properties content view-model.
- The progress selector now mirrors the active note header's `progress enums="{...}"` order before it
  mirrors any injected Progress hierarchy source, so the detail panel stops presenting an unrelated
  standalone taxonomy.
- When a Progress hierarchy source is injected, the selector still reuses matching source metadata
  such as `itemId`, `progressValue`, and icon data, so writes keep the owning hierarchy's numeric
  mapping instead of collapsing back to plain row indices.
- Progress persistence resolves the enum integer from hierarchy entry metadata such as `itemId` or a numeric `progress:*` key; it is no longer hard-coded to the default `Ready/Pending/InProgress/Done` labels.
- Clearing progress writes the `.wsnhead` field as an explicit empty progress value, which round-trips back into the detail selector as `No progress`.
- The properties form now mirrors `header.folders()` directly, so folder rows stay aligned with the persisted `.wsnhead` path strings instead of showing only the leaf segment.
