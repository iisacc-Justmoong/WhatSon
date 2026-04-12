# `src/app/qml/view/panels/detail/DetailPanelHeaderToolbar.qml`

## Responsibility
`DetailPanelHeaderToolbar.qml` renders the six-button detail toolbar for Figma frame `155:4575`.
It normalizes toolbar specs, sizes itself from the resolved button row, and emits `detailStateChangeRequested(int)` when a button is activated.

## Visual Contract
- Root `objectName`: `DetailPanelHeaderToolbar`
- Figma node id: `155:4575`
- Frame size: derived from the internal `Row` implicit size
- Inter-button spacing: `LV.Theme.scaleMetric(5)`
- The file must import `LVRS 1.0 as LV` directly because it reads `LV.Theme` at the root scope.
- The toolbar is centered by its parent panel, while this file only preserves the internal row geometry.
- The toolbar must not reintroduce a fixed `145x20` desktop clamp, because that makes the mobile detail header appear unnaturally small.

## Delegate Contract
Each delegate receives a normalized spec object with:
- `figmaNodeId`
- `iconName`
- `objectName`
- `stateValue`
- `selected`

The toolbar itself remains data-driven and does not hardcode state transitions beyond emitting the selected state value.
