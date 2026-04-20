# `src/app/qml/view/content/editor/ContentsEditorSurfaceModeSupport.js`

## Responsibility

Provides one small QML-side policy helper that decides whether the center content slot should mount the note editor or
the dedicated resource editor.

## Public Helpers

- `resourceEditorVisible(noteListModel)`
  Returns `true` only when the active list model exposes a `currentResourceEntry` payload and is not explicitly
  note-backed.
- `currentResourceEntry(noteListModel)`
  Returns the currently selected resource entry only when the active list model is in resource-editor mode.
- `hasCurrentResourceEntry(noteListModel)`
  Convenience probe for callers that only need a boolean.

## Notes

- This helper keeps the resource-surface routing rule in one place instead of open-coding it in
  `ContentViewLayout.qml`.
- The contract intentionally treats `noteBacked === true` as authoritative, because note hierarchies may still expose
  inline resource presentation inside ordinary note bodies without switching away from the note editor.
- The helper is read-side only.
  It does not mutate selection state or persistence state.

## Tests

- Automated regression coverage executes this helper through `QJSEngine` in
  `test/cpp/suites/*.cpp`.
- Regression checklist:
  - direct resource list models must switch the center surface into resource-editor mode
  - note-backed list models must keep the note editor even if a helper payload is present
  - callers that request the current resource entry outside resource mode must receive an empty object
