# `src/app/qml/view/panels/ResourceListItem.qml`

## Responsibility

`ResourceListItem.qml` is the dedicated right-panel card component for resources-list rows. It intentionally does not
reuse `NoteListItem` structure and keeps the Figma `232:7892` geometry and state colors as an isolated contract.

## Source Metadata
- Source path: `src/app/qml/view/panels/ResourceListItem.qml`
- Source kind: QML view/component
- File name: `ResourceListItem.qml`
- Approximate line count: 91

## QML Surface Snapshot
- Root type: `Item`

### Object IDs
- `resourceListItem`
- `resourceHoverHandler`

### Public Properties
- `active`
- `pressed`
- `previewSource`
- `titleText`

## Visual Contract

- Width uses `LV.Theme.scaleMetric(194)` instead of a raw fixed pixel literal.
- Frame padding uses `LV.Theme.gap8`.
- Single horizontal row with:
  - a thumbnail frame sized from `LV.Theme.scaleMetric(48)`.
  - a row gap sized from `LV.Theme.scaleMetric(10)`.
  - semibold title text with `12px` design metrics routed through `LV.Theme.scaleMetric(...)`.
- Background states:
  - default: transparent
  - hover/pressed: `LV.Theme.panelBackground06`
  - active: `#25324D`

## Integration

- `ListBarLayout.qml` switches delegate composition by `resourceListMode`.
- Resources rows render `ResourceListItem`; non-resource note rows keep `NoteListItem`.

## Tests

Automated test files were removed from this repository; verify component geometry and delegate bindings through runtime inspection.
