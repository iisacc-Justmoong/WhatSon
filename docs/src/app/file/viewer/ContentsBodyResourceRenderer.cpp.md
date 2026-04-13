# `src/app/file/viewer/ContentsBodyResourceRenderer.cpp`

## Responsibility
Implements note-body resource rendering data extraction for the editor surface.

## Key Behavior
- Resolves the selected note directory from `contentViewModel.noteDirectoryPathForNoteId(noteId)`.
- If the active hierarchy view-model cannot resolve that note directory, it retries through
  `fallbackContentViewModel.noteDirectoryPathForNoteId(noteId)`.
  Desktop/mobile bind this fallback to `LibraryHierarchyViewModel` so inline resource rendering survives hierarchy
  transition lag and domains such as tags that do not own their own note-directory resolver.
- If the resolved path is a `.wsresource` directory, it renders that package directly as a single
  resource card.
- Otherwise it prefers the live `bodySourceText` snapshot supplied by the editor host and scans that RAW source for
  `<resource ...>` tags, falling back to the active `.wsnbody` file only when the host has no same-note presentation
  snapshot yet.
- Supports quoted and unquoted attribute values (including package paths with `/`).
- Scans comments and resource tags in one pass so comment text does not rewrite the authoring buffer before source-span
  offsets are recorded.
- Resolves `.wsresource` references through `WhatSon::Resources::resolveAssetLocationFromReference(...)`.
- Produces render entries with:
  - `renderMode = "image"` for image resources
  - `renderMode = "video"` for video resources
  - `renderMode = "audio"` for audio/music resources
  - `renderMode = "pdf"` for pdf resources
  - `renderMode = "text"` for previewable text resources (`previewText` snippet included)
  - `renderMode = "document"` fallback for remaining resource types
- Exposes source-span metadata (`sourceStart`, `sourceEnd`, `focusSourceOffset`) for each note-body tag so QML can
  place resource cards at the authored inline position instead of pinning them to one bottom overlay rail.
- Rebuilds the render model when the selected note changes or when `hubFilesystemMutated()` is emitted by the content view-model.
- Rebuilds the render model when either the primary or fallback resolver emits `hubFilesystemMutated()`.
- Rebuilds the render model immediately when `bodySourceText` changes, which lets drag-inserted resource tags render
  before the background note-persistence worker finishes.

## Testing
  - `resourceRenderer_mustResolveResourceTagsFromCurrentNoteBody`
  - `resourceRenderer_mustRenderDirectResourcePackageSelection`
