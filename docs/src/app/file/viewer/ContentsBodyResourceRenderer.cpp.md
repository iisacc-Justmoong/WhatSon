# `src/app/file/viewer/ContentsBodyResourceRenderer.cpp`

## Responsibility
Implements note-body resource rendering data extraction for the editor surface.

## Key Behavior
- Resolves the selected note directory from `contentViewModel.noteDirectoryPathForNoteId(noteId)`.
- If the resolved path is a `.wsresource` directory, it renders that package directly as a single
  resource card.
- Otherwise it loads the active `.wsnbody` file and scans `<resource ...>` tags inside `<body>`.
- Supports quoted and unquoted attribute values (including package paths with `/`).
- Resolves `.wsresource` references through `WhatSon::Resources::resolveAssetLocationFromReference(...)`.
- Produces render entries with:
  - `renderMode = "image"` for image resources
  - `renderMode = "video"` for video resources
  - `renderMode = "audio"` for audio/music resources
  - `renderMode = "pdf"` for pdf resources
  - `renderMode = "text"` for previewable text resources (`previewText` snippet included)
  - `renderMode = "document"` fallback for remaining resource types
- Rebuilds the render model when the selected note changes or when `hubFilesystemMutated()` is emitted by the content view-model.

## Testing
- Covered by `tests/app/test_contents_editor_bridge.cpp`:
  - `resourceRenderer_mustResolveResourceTagsFromCurrentNoteBody`
  - `resourceRenderer_mustRenderDirectResourcePackageSelection`
