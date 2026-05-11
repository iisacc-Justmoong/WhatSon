# `src/app/models/file/note/support`

## Scope
- Owns shared support code that is specific to the note file domain.
- Currently provides the iiXml document-tree boundary used by header and body package readers.

## Files
- `WhatSonIiXmlDocumentSupport.*`

## Boundary
- May centralize low-level note XML traversal and attribute/text extraction.
- Must not own domain mutation, persistence transactions, or UI orchestration.
