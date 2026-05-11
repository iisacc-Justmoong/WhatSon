# `src/app/models/file/note/package`

## Scope
- Owns base note package creation helpers and initial body package creation.
- Keeps package bootstrap rules separate from ongoing body persistence and hub mutation services.

## Files
- `WhatSonNoteBodyCreator.*`
- `WhatSonNoteCreator.*`

## Boundary
- May compose header/body defaults and package paths for a newly created note.
- Must not own selected-note session orchestration or post-create hierarchy selection state.
