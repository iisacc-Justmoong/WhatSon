# `src/app/models/file/note/package`

## Scope
- Owns base note package creation helpers.
- Keeps package bootstrap rules separate from hub mutation services.

## Files
- `WhatSonNoteCreator.*`

## Boundary
- May compose header defaults and package paths for a newly created note.
- Must not own selected-note session orchestration, body persistence, or post-create hierarchy selection state.
