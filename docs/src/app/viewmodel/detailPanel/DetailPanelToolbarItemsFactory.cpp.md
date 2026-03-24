# `src/app/viewmodel/detailPanel/DetailPanelToolbarItemsFactory.cpp`

## Responsibility
This factory builds the canonical toolbar spec list consumed by `DetailPanelViewModel`.
It is the C++ source of truth for the six detail-panel state buttons and their icon names.

## Current Mapping
- `Properties` -> `config`
- `FileStat` -> `chartBar`
- `Insert` -> `generaladd`
- `Layer` -> `toolwindowdependencies`
- `FileHistory` -> `toolWindowClock`
- `Help` -> `featureAnswer`

## Output Shape
Each generated toolbar item is a `QVariantMap` with:
- `iconName`
- `stateValue`
- `selected`

## Notes
- The first icon changed from the old project-structure glyph to `config` so the desktop detail toolbar matches the Figma `Properties` button contract.
- QML still enriches these items with Figma node ids and semantic object names for inspection and testing.
