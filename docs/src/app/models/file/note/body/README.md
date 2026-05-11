# `src/app/models/file/note/body`

## Scope
- Owns `.wsnbody` parsing, persistence normalization, semantic body tags, raw resource tags, web-link canonicalization, and body-local markdown style objects.
- Keeps body-format policy close to the note package store while avoiding hub orchestration or file-system mutation responsibilities.

## Files
- `WhatSonNoteBodyPersistence.*`
- `WhatSonNoteBodyResourceTagGenerator.*`
- `WhatSonNoteBodySemanticTagSupport.*`
- `WhatSonNoteBodyWebLinkSupport.*`
- `WhatSonNoteMarkdownStyleObject.*`

## Boundary
- Depends on note-local XML support and package file-store APIs.
- Must not own note creation, hub selection, folder binding, or UI session orchestration.
