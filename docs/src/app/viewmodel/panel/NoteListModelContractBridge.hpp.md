# `src/app/viewmodel/panel/NoteListModelContractBridge.hpp`

## Responsibility

`NoteListModelContractBridge` is a QML-facing adapter that moves dynamic note-list contract probing out of
`ListBarLayout.qml` and into C++.

## Public Contract

- Accepts either:
  - an explicit `QObject*` `noteListModel`, or
  - a `QObject*` `hierarchyViewModel` whose `hierarchyNoteListModel` / `noteListModel` contract identifies the active
    list model for the current hierarchy domain.
- Explicit `noteListModel` input still wins when both are present, so existing callers can override the automatic
  hierarchy-derived selection path when needed.
- Exposes explicit capability flags:
  - `hasNoteListModel`
  - `searchContractAvailable`
  - `currentIndexContractAvailable`
- Exposes normalized read contracts:
  - `currentIndex`
  - `currentNoteId`
  - `readNoteIdAt(int)` for turning visual multi-selection row indexes back into stable note ids
  - `readAllRows()` for exporting the current list rows as role-name keyed snapshots that QML can diff without binding
    directly to a resetting `QAbstractItemModel`
  - `readAllRowsForModel(QObject*)` for exporting the same snapshot contract against an explicit model object during
    hierarchy transitions, before the bridge's own `noteListModel` binding has necessarily settled
- Exposes write helpers:
  - `applySearchText(QString)`
  - `pushCurrentIndex(int)`

## Why It Exists

`ListBarLayout.qml` must still support multiple hierarchy note-list models with different runtime surfaces.
This bridge centralizes dynamic property/method detection so the QML side can stay focused on rendering and interaction
flow rather than reflection-heavy contract logic.

The same bridge also lets desktop/mobile shells hand over the resolved hierarchy viewmodel, the resolved
`activeNoteListModel`, or both during toolbar/domain switches. Current shell wiring prefers forwarding the explicit
note-list model so the list surface and editor surface consume the same object on the same turn, while the
hierarchy-derived path remains as the fallback for callers that only know the hierarchy object.
