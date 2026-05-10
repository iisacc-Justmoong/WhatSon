# `src/app/models/panel/NoteActiveStateTracker.cpp`

## Responsibility

Implements global active-note tracking by subscribing to the active hierarchy context and the currently active
note-list model.
It deliberately stops at selection and file-path publication; editor-session mounting, projection, and rendering are no
longer responsibilities of this object.

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
- `bodyText` row data is intentionally omitted from `activeNoteEntry`; the same note-list body value is stored
  separately as `activeNoteBodyText` for read-side selection context.
- `activeNoteBodyPath` is derived from `activeNoteDirectoryPath` through `WhatSon::NoteBodyPersistence`.
- `setActiveNoteState(...)` commits the next entry, note id, note directory path, body path, and body text before emitting any
  change signal. Synchronous observers must never see a new `activeNoteId` paired with the previous note's
  `activeNoteBodyPath` or `activeNoteBodyText`.

## Tests

Covered by `test/cpp/suites/note_active_state_tracker_tests.cpp` and architecture-lock checks in
`test/cpp/suites/architecture_policy_lock_tests.cpp`.

## 한국어

- 대상: `src/app/models/panel/NoteActiveStateTracker.cpp`
- 역할: active hierarchy와 active note-list model의 변화를 구독해 전역 active note 상태를 갱신한다.
- 검증: active hierarchy 전환, 빈 selection clear, `noteBacked=false` clear, architecture lock 이후 재배선 거부를
  회귀 테스트로 고정한다. change signal 중에도 note id/path/body file/body snapshot이 원자적으로 일치하는지도 회귀 테스트로
  고정한다.
