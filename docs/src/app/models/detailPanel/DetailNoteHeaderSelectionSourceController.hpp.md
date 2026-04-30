# `src/app/models/detailPanel/DetailNoteHeaderSelectionSourceController.hpp`

## Role
`DetailNoteHeaderSelectionSourceController` builds selectable project/bookmark/progress models for the detail panel.

## Interface Alignment
- Session access now depends on `IWhatSonNoteHeaderSessionStore`.
- The controller remains free to consume any compatible header-session implementation.
