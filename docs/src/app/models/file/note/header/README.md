# `src/app/models/file/note/header`

## Scope
- Owns `.wsnhead` creation, parsing, storage, and header-local presentation metadata such as bookmark colors.
- Keeps header schema work isolated from body persistence and hub mutation services.

## Files
- `WhatSonBookmarkColorPalette.hpp`
- `WhatSonNoteHeaderCreator.*`
- `WhatSonNoteHeaderParser.*`
- `WhatSonNoteHeaderStore.*`

## Boundary
- Uses shared XML support for tag and attribute extraction.
- Must not own `.wsnbody` serialization, folder binding state, or editor session lifecycle.
