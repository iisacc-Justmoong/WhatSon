# `src/app/file/viewer/ContentsBodyResourceRenderer.cpp`

## Responsibility
Implements note-body resource rendering data extraction for the editor surface.

## Key Behavior
- Resolves the selected note directory from `contentViewModel.noteDirectoryPathForNoteId(noteId)`.
- If the resolved path is a `.wsresource` directory, it renders that package directly as a single
  resource card.
- Otherwise it prefers the live `bodySourceText` snapshot supplied by the editor host and scans that RAW source for
  `<resource ...>` tags, falling back to the active `.wsnbody` file only when the host has no same-note presentation
  snapshot yet.
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
- Rebuilds the render model immediately when `bodySourceText` changes, which lets drag-inserted resource tags render
  before the background note-persistence worker finishes.

## Testing
  - `resourceRenderer_mustResolveResourceTagsFromCurrentNoteBody`
  - `resourceRenderer_mustRenderDirectResourcePackageSelection`
