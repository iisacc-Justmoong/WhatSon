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

- Fixed width `194px`.
- `8px` frame padding around content.
- Single horizontal row with:
  - `48x48` thumbnail frame (`imageBox` placeholder).
  - `10px` gap.
  - semibold `12px` title text block.
- Background states:
  - default: transparent
  - hover/pressed: `LV.Theme.panelBackground06`
  - active: `#25324D`

## Integration

- `ListBarLayout.qml` switches delegate composition by `resourceListMode`.
- Resources rows render `ResourceListItem`; non-resource note rows keep `NoteListItem`.

## Tests

- `tests/app/test_qml_binding_syntax_guard.cpp` verifies:
  - component existence,
  - fixed geometry and state-color contracts,
  - thumbnail/title bindings from delegate model roles.
