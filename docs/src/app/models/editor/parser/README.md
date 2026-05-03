# `src/app/models/editor/parser`

## Status
- Directory mirror generated from the current `src` tree.
- This file is the entry point for the detailed documentation pass of this directory.

## Scope
- Mirrored source directory: `src/app/models/editor/parser`
- Child directories: 0
- Child files: 2

## Child Directories
- No child directories.

## Child Files
- `ContentsWsnBodyBlockParser.cpp`
- `ContentsWsnBodyBlockParser.hpp`

## Current Notes
- `ContentsWsnBodyBlockParser` is the new editor-side read parser for `.wsnbody`.
- It now prefers an `iiXml::Parser::TagParser` document tree for supported explicit body tags, then emits one ordered
  `renderedDocumentBlocks` list for the structured QML editor host.
- The older token scan is retained only as a recovery lane for transient malformed edit states that cannot yet produce an
  iiXml tree.
- Agenda/callout payloads are now also derived in that same pass instead of being reparsed by independent read-side
  backends before the renderer can merge them.
- Explicit semantic text blocks now expose:
  - wrapper geometry through `blockSourceStart` / `blockSourceEnd` plus open/close-tag offsets
  - editable content geometry through `sourceStart` / `sourceEnd` / `sourceText`
  This keeps `ContentsDocumentTextBlock.qml` on an inner-content editing contract even when the authored RAW still
  uses wrapper tags such as `<paragraph>...</paragraph>`.
- Every emitted block now also carries one generic document-block trait payload for the QML flow host:
  `plainText`, `textEditable`, `atomicBlock`, `gutterCollapsed`, `logicalLineCountHint`,
  `minimapVisualKind`, and `minimapRepresentativeCharCount`.
- The repository still does not maintain an in-repo automated editor test suite, so parser regression expectations are
  documented here rather than enforced by local tests.
