# `src/app/editor/renderer/ContentsBodyResourceRenderer.cpp`

## Responsibility
Implements note-body resource rendering data extraction for the editor surface.

## Key Behavior
- Resolves the selected note directory from `contentViewModel.noteDirectoryPathForNoteId(noteId)`.
- Loads the active `.wsnbody` file and scans `<resource ...>` tags inside `<body>`.
- Supports quoted and unquoted attribute values (including package paths with `/`).
- Resolves `.wsresource` references through `WhatSon::Resources::resolveAssetLocationFromReference(...)`.
- Produces render entries with:
  - `renderMode = "image"` for resolvable image resources
  - `renderMode = "unsupported"` for non-image or unresolved entries
- Rebuilds the render model when the selected note changes or when `hubFilesystemMutated()` is emitted by the content view-model.

## Testing
- Covered by `tests/app/test_contents_editor_bridge.cpp` (`resourceRenderer_mustResolveResourceTagsFromCurrentNoteBody`).
