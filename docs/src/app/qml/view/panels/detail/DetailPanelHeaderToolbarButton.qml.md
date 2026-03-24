# `src/app/qml/view/panels/detail/DetailPanelHeaderToolbarButton.qml`

## Responsibility
`DetailPanelHeaderToolbarButton.qml` is the per-button delegate used by the detail toolbar.
It adapts toolbar spec data into an `LV.IconButton` while preserving the Figma object name and node id for inspection/debugging.

## Contract
- Root type: `LV.IconButton`
- Fixed frame: `20 x 20`
- Icon size: `16`
- Selected tone: `LV.AbstractButton.Default`
- Unselected tone: `LV.AbstractButton.Borderless`

## Data Inputs
- `buttonSpec.iconName`
- `buttonSpec.objectName`
- `buttonSpec.figmaNodeId`
- `buttonSpec.selected`
- `buttonSpec.stateValue`

## Behavior
- Invalid or non-numeric `stateValue` is ignored and logged through the existing view-hook path.
- Valid clicks emit `stateClickRequested(nextState)`.
