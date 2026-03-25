# `src/app/viewmodel/detailPanel/DetailPanelViewModel.cpp`

## Responsibility
`DetailPanelViewModel.cpp` owns the active detail-panel state, the active section view-model pointer, the toolbar selection list, and the three selector-copy objects used by the properties form.

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
- `main.cpp` injects the canonical Projects/Bookmarks/Progress hierarchy viewmodels only as source objects for these copies.
