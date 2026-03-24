# `src/app/qml/view/panels/detail/DetailPanelHeaderToolbar.qml`

## Responsibility
`DetailPanelHeaderToolbar.qml` renders the six-button detail toolbar for Figma frame `155:4575`.
It normalizes toolbar specs, keeps the `145x20` Figma frame, and emits `detailStateChangeRequested(int)` when a button is activated.

## Visual Contract
- Root `objectName`: `DetailPanelHeaderToolbar`
- Figma node id: `155:4575`
- Frame size: `145 x 20`
- Inter-button spacing: `5`
- The toolbar is centered by its parent panel, while this file only preserves the internal row geometry.

## Delegate Contract
Each delegate receives a normalized spec object with:
- `figmaNodeId`
- `iconName`
- `objectName`
- `stateValue`
- `selected`

The toolbar itself remains data-driven and does not hardcode state transitions beyond emitting the selected state value.
