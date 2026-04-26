# `src/app/models/editor/session`

## Responsibility
Owns the C++ editor session controller.

## Current Modules
- `ContentsEditorSessionController.*`
  C++ session authority for pending saves, same-note echo acceptance, and synchronized editor text state.

## Boundary
- Editor hosts must instantiate `ContentsEditorSessionController` directly through the internal C++ QML type.
- This directory must not contain QML wrappers. QML is reserved for view construction and must not carry editor
  session, persistence, or synchronization policy.
- Persistence decisions must still route through RAW `.wsnote/.wsnbody` mutation and sync paths.
