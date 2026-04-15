# `src/app/file/viewer/ContentsBodyResourceRenderer.cpp`

## Responsibility
Implements note-body resource rendering data extraction for the editor surface.

## Key Behavior
- Prefers an explicit `noteDirectoryPath` from the editor-selection bridge when QML provides it.
  This lets the renderer resolve `.wsresource` package references from the same note directory that the editor session
  already bound, instead of waiting for the currently active hierarchy view-model to answer
  `noteDirectoryPathForNoteId(...)` again.
- Resolves the selected note directory from `contentViewModel.noteDirectoryPathForNoteId(noteId)`.
- If the active hierarchy view-model cannot resolve that note directory, it retries through
  `fallbackContentViewModel.noteDirectoryPathForNoteId(noteId)`.
  Desktop/mobile bind this fallback to `LibraryHierarchyViewModel` so inline resource rendering survives hierarchy
  transition lag and domains such as tags that do not own their own note-directory resolver.
- If the resolved path is a `.wsresource` directory, it renders that package directly as a single
  resource card.
- Otherwise it consumes the parser-owned `documentBlocks` stream supplied by
  `ContentsStructuredBlockRenderer.renderedDocumentBlocks` and resolves only `type=resource` blocks.
  The renderer no longer reparses `.wsnbody` with its own `<resource ...>` grammar.
- Resolves `.wsresource` references through `WhatSon::Resources::resolveAssetLocationFromReference(...)`.
- Expands resource-reference base paths beyond the mounted note directory itself:
  note-directory ancestors up to the owning `.wshub`, the resolved `.wscontents` directory, the hub parent for
  legacy `Hub.wshub/...` references, and every discovered `*.wsresources` root now participate in path resolution.
  Body `<resource ... />` tags therefore resolve consistently whether they were authored relative to the note package,
  a contents-level directory, the hub root, or a resource-root directory.
- When a `<resource ... />` tag points at a `.wsresource` package, the renderer now reloads that package's
  `resource.xml` metadata during body rendering and promotes `resourceId`, `resourcePath`, `type`, and `format` from
  metadata before building the QML-facing entry.
  Inline image promotion therefore no longer depends solely on the literal tag attributes staying perfectly in sync
  with package metadata.
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
- Rebuilds the render model immediately when parser-owned `documentBlocks` changes, so same-note RAW rewrites and
  reparses update the resolved inline resource payload on the same editor turn.

## Testing
  - `resourceRenderer_mustResolveResourceBlocksFromStructuredDocumentBlocks`
  - `resourceRenderer_mustRenderDirectResourcePackageSelection`
