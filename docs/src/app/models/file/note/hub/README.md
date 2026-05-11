# `src/app/models/file/note/hub`

## Scope
- Owns hub-scoped note mutations: creation, deletion, folder clearing, and shared mutation support.
- Keeps workspace package mutation flows separate from local package parsing helpers.

## Files
- `WhatSonHubNoteCreationService.*`
- `WhatSonHubNoteDeletionService.*`
- `WhatSonHubNoteFolderClearService.*`
- `WhatSonHubNoteMutationSupport.*`

## Boundary
- Coordinates note package changes in the context of a mounted hub.
- Must not own body tag serialization, header parsing, or content-view session state.
