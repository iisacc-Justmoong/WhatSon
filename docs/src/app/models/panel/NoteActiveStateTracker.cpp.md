# `src/app/models/panel/NoteActiveStateTracker.cpp`

## Responsibility

Implements global active-note tracking by subscribing to the active hierarchy context and the currently active
note-list model.
It also synchronizes the attached `ContentsEditorSessionController` from the active note snapshot, so note selection and
editor-session mounting happen in the same C++ update path.

## Behavior Summary

- `setHierarchyContextSource(...)` accepts the sidebar-level `IActiveHierarchyContextSource` before the architecture
  policy lock and rejects rewiring after the lock.
- `synchronizeActiveBindings()` refreshes the active hierarchy index, active hierarchy controller, and active note-list
  model as one snapshot.
- The active note-list model is observed for:
  - `currentIndexChanged()`
  - `currentNoteEntryChanged()`
  - `currentNoteIdChanged()`
  - `currentNoteDirectoryPathChanged()`
  - `currentBodyTextChanged()`
  - `noteBackedChanged()`
  - relevant `QAbstractItemModel` row/reset/layout changes
- Active note resolution prefers `currentNoteEntry`, then `currentNoteId/currentNoteDirectoryPath`, and only falls back
  to current-row role snapshots when the model has no committed note-id contract.
- `bodyText` row data is intentionally omitted from `activeNoteEntry`; the same RAW body snapshot is stored separately
  as `activeNoteBodyText` and is used only to mount the attached editor session.
- `setEditorSession(...)` accepts the visible `ContentsEditorSessionController`. When active note identity or body text
  changes, `syncEditorSessionFromActiveNote()` calls
  `requestSyncEditorTextFromSelection(activeNoteId, activeNoteBodyText, activeNoteId, activeNoteDirectoryPath)`.

## Tests

Covered by `test/cpp/suites/note_active_state_tracker_tests.cpp`, QML wiring checks in
`test/cpp/suites/qml_editor_surface_policy_tests.cpp`, and architecture-lock checks in
`test/cpp/suites/architecture_policy_lock_tests.cpp`.

## 한국어

- 대상: `src/app/models/panel/NoteActiveStateTracker.cpp`
- 역할: active hierarchy와 active note-list model의 변화를 구독해 전역 active note 상태를 갱신하고, 현재 표시 중인
  편집기 세션을 즉시 갱신한다.
- 검증: active hierarchy 전환, 빈 selection clear, `noteBacked=false` clear, architecture lock 이후 재배선 거부를
  회귀 테스트로 고정한다. active note 변경과 같은 턴에 세션이 갱신되는지도 회귀 테스트로 고정한다.
