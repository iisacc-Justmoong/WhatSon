# `src/app/models/panel/NoteActiveStateTracker.hpp`

## Responsibility

`NoteActiveStateTracker` is the app-wide, QML-facing active-note state object.

It observes one `IActiveHierarchyContextSource`, follows its current active note-list model, and republishes the
normalized note selection as a single stable object contract. It also accepts the currently mounted
`ContentsEditorSessionController` so the editor session can be synchronized from the same active-note turn that changed
the selection.

## Public Contract

- Inputs:
  - `hierarchyContextSource`
  - `editorSession`
- Active hierarchy snapshot:
  - `activeHierarchyIndex`
  - `activeHierarchyController`
  - `activeNoteListModel`
- Active note snapshot:
  - `activeNoteEntry`
  - `activeNoteId`
  - `activeNoteDirectoryPath`
  - `activeNoteBodyText`
  - `hasActiveNote`
- Invokable operations:
  - `syncEditorSessionFromActiveNote()`
- Signals:
  - per-property change signals
  - `activeNoteStateChanged()` for consumers that need one composite note-state invalidation hook

## Invariants

- The tracker does not mutate note-list selection, note source, or persistence state.
- The tracker may ask the attached `ContentsEditorSessionController` to mount the active note's RAW body snapshot. This
  keeps editor-session rebinding synchronous with active-note selection instead of waiting for later QML binding turns.
- A note-list model with readable `noteBacked == false` clears the active-note identity even if it exposes
  `currentNoteId`.
- A readable-but-empty `currentNoteEntry` / `currentNoteId` contract clears the active note instead of preserving stale
  identity.
- `activeNoteDirectoryPath` is normalized with `QDir::cleanPath` and never publishes `"."`.
- `bodyText` is not published inside `activeNoteEntry`; the RAW body snapshot is exposed separately as
  `activeNoteBodyText` for editor-session mounting.

## 한국어

- 대상: `src/app/models/panel/NoteActiveStateTracker.hpp`
- 역할: 사이드바의 active hierarchy context를 따라 현재 active note 상태를 전역 QML 객체 하나로 노출하고, 붙어 있는
  편집기 세션을 같은 턴에 동기화한다.
- 기준: note-list selection과 note source는 수정하지 않는다. 세션 마운트만 active note 스냅샷에서 즉시 갱신한다.
