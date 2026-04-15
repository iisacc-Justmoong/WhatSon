# `src/app/editor/parser`

## Status
- Directory mirror generated from the current `src` tree.
- This file is the entry point for the detailed documentation pass of this directory.

## Scope
- Mirrored source directory: `src/app/editor/parser`
- Child directories: 0
- Child files: 2

## Child Directories
- No child directories.

## Child Files
- `ContentsWsnBodyBlockParser.cpp`
- `ContentsWsnBodyBlockParser.hpp`

## Current Notes
- `ContentsWsnBodyBlockParser` is the new editor-side read parser for `.wsnbody`.
- It walks the canonical RAW source once, recognizes supported top-level body tags, and emits one ordered
  `renderedDocumentBlocks` list for the structured QML editor host.
- Agenda/callout payloads are now also derived in that same pass instead of being reparsed by independent read-side
  backends before the renderer can merge them.
- Explicit semantic text blocks now expose:
  - wrapper geometry through `blockSourceStart` / `blockSourceEnd` plus open/close-tag offsets
  - editable content geometry through `sourceStart` / `sourceEnd` / `sourceText`
  This keeps `ContentsDocumentTextBlock.qml` on an inner-content editing contract even when the authored RAW still
  uses wrapper tags such as `<paragraph>...</paragraph>`.
- The repository still does not maintain an in-repo automated editor test suite, so parser regression expectations are
  documented here rather than enforced by local tests.
