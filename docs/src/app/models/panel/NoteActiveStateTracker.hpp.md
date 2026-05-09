# `src/app/models/panel/NoteActiveStateTracker.hpp`

## Responsibility

`NoteActiveStateTracker` is the app-wide, QML-facing active-note state object.

It observes one `IActiveHierarchyContextSource`, follows its current active note-list model, and republishes the
normalized note selection as a single stable object contract. It no longer owns editor-session synchronization; the
LVRS `TextEditor` surface is mounted independently from the removed editor backend stack.

## Public Contract

- Inputs:
  - `hierarchyContextSource`
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
- Signals:
  - per-property change signals
  - `activeNoteStateChanged()` for consumers that need one composite note-state invalidation hook

## Invariants

- The tracker does not mutate note-list selection, note source, or persistence state.
- A note-list model with readable `noteBacked == false` clears the active-note identity even if it exposes
  `currentNoteId`.
- A readable-but-empty `currentNoteEntry` / `currentNoteId` contract clears the active note instead of preserving stale
  identity.
- `activeNoteDirectoryPath` is normalized with `QDir::cleanPath` and never publishes `"."`.
- `bodyText` is not published inside `activeNoteEntry`; the note-list body value is exposed separately as
  `activeNoteBodyText` for read-side UI consumers that still need selection context.

## 한국어

- 대상: `src/app/models/panel/NoteActiveStateTracker.hpp`
- 역할: 사이드바의 active hierarchy context를 따라 현재 active note 상태를 전역 QML 객체 하나로 노출한다.
- 기준: note-list selection, note source, editor session, persistence를 수정하지 않는다.
