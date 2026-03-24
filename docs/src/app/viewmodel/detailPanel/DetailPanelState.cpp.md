# `src/app/viewmodel/detailPanel/DetailPanelState.cpp`

## Responsibility
`DetailPanelState.cpp` maps the internal detail-panel enum values to externally visible state ids.

## Current State Id Contract
- `Properties` -> `properties`
- `FileStat` -> `fileStat`
- `Insert` -> `insert`
- `FileHistory` -> `fileHistory`
- `Layer` -> `layer`
- `Help` -> `help`

## Notes
- The enum names themselves follow the exported Figma page identifiers.
- QML uses these ids to choose toolbar pages and content forms.
