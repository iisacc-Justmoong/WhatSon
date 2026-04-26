# `src/app/models/editor/session`

## Responsibility
Owns editor session QML wrappers and C++ session controllers.

## Current Modules
- `ContentsEditorSessionController.*`
  C++ session authority for pending saves, same-note echo acceptance, and synchronized editor text state.
- `ContentsEditorSession.qml`
  Compatibility QML wrapper for older QML call sites that still expect the session object shape.

## Boundary
- New editor hosts should prefer `ContentsEditorSessionController` directly.
- Persistence decisions must still route through RAW `.wsnote/.wsnbody` mutation and sync paths.
