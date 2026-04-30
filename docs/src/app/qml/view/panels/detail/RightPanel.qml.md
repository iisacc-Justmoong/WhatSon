# `src/app/qml/view/panels/detail/RightPanel.qml`

## Responsibility
`RightPanel.qml` is the outer desktop detail-panel shell for the Figma `RightPanel` frame (`155:4574`).
It keeps the panel canvas transparent, preserves the panel-view-model hook entrypoint, and mounts the actual detail composition through `DetailPanel.qml`.

## Visual Contract
- Figma root frame id: `155:4574`
- Root `objectName`: `RightPanel`
- Default/min panel widths now resolve through named `LV.Theme` width/gap/stroke token compositions.
- The child `DetailPanel` fills the wrapper, so parent layout sizing controls the final width.

## Runtime Notes
- The file intentionally stays thin.
- It does not own detail state; it only forwards lifecycle visibility through the shared panel wrapper pattern already used in the rest of the desktop shell.
- `requestViewHook(reason)` still routes through `panelControllerRegistry.panelController("detail.RightPanel")`.

## Integration
- Parent wrapper: `src/app/qml/view/panels/DetailPanelLayout.qml`
- Child composition: `src/app/qml/view/panels/detail/DetailPanel.qml`
