# `src/app/models/file/note/session`

## Scope
- Owns content-panel note management orchestration that bridges selected-note state, package persistence, and follow-up metadata refresh work.
- Keeps view-facing note management lifecycle code separate from local package parsing and hub mutation helpers.

## Files
- `ContentsNoteManagementCoordinator.*`

## Boundary
- May coordinate note selection, header-only open-count updates, body reads, and post-persist stat refresh.
- Must not implement raw body serialization policy or reusable XML parsing helpers.
