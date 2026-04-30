# `src/app/models/editor/session`

## Responsibility
Owns the C++ editor session controller.

## Current Modules
- `ContentsEditorSessionController.*`
  C++ session authority for pending saves, same-note echo acceptance, synchronized editor text state, and accepted local
  RAW source commits.

## Boundary
- Editor hosts must instantiate `ContentsEditorSessionController` directly through the internal C++ QML type.
- This directory must not contain QML wrappers. QML is reserved for view construction and must not carry editor
  session, persistence, or synchronization policy.
- Local editor mutations must enter through `commitRawEditorTextMutation(...)`; QML controllers may build candidate RAW
  text, but they must not write `editorText`, mark local authority, or schedule persistence themselves.
- Explicit tag-management mutations may follow that session commit with
  `persistEditorTextImmediatelyWithText(...)` so discrete RAW tag insertions are flushed through the same persistence
  boundary without waiting for the idle typing path.
- Persistence decisions must still route through RAW `.wsnote/.wsnbody` mutation and sync paths.
