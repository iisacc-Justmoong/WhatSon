# `src/app/models/content/display`

## Responsibility
`src/app/models/content/display` owns transient editor-host coordination for selection, context-menu routing, mount
planning, and display refresh behavior.

## Notes
- Geometry-heavy viewport and minimap math has moved to `src/app/models/editor/display` so `ContentsDisplayView.qml`
  can delegate more calculations into C++.
- Context-menu coordination remains here because it still belongs to display interaction policy rather than editor
  document storage or parsing authority.
- Note-body mount policy must continue to accept explicit empty-body snapshots owned by the selected note; a blank
  note is a valid mounted document, not a mount failure.
