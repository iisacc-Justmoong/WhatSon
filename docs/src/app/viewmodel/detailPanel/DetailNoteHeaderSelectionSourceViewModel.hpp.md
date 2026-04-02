# `src/app/viewmodel/detailPanel/DetailNoteHeaderSelectionSourceViewModel.hpp`

## Role
`DetailNoteHeaderSelectionSourceViewModel` builds selectable project/bookmark/progress models for the detail panel.

## Interface Alignment
- Session access now depends on `IWhatSonNoteHeaderSessionStore`.
- The viewmodel remains free to consume any compatible header-session implementation.
