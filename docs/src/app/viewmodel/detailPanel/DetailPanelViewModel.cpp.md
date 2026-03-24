# `src/app/viewmodel/detailPanel/DetailPanelViewModel.cpp`

## Responsibility
`DetailPanelViewModel.cpp` owns the active detail-panel state, the active section view-model pointer, and the toolbar selection list.

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
