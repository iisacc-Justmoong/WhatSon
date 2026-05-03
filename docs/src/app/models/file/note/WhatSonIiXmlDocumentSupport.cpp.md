# `src/app/models/file/note/WhatSonIiXmlDocumentSupport.cpp`

## Responsibility

Implements the shared note-package adapter around the local iiXml parser.

## Key Behavior

- Strips XML declarations and top-level doctype preambles before invoking `iiXml::Parser::TagParser`.
- Converts iiXml `std::string_view` slices into `QString` while preserving the parser-owned document source copy.
- Decodes common XML entities for node text and attribute values.
- Traverses parsed `TagNode` trees case-insensitively for note-package lookups.
- Provides first-non-empty attribute resolution across alternate attribute names, which lets older resource tag shapes
  continue to load without reintroducing local regex scans.
