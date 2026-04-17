# `src/app/file/note/WhatSonNoteBodyResourceTagGenerator.cpp`

## Responsibility
Owns the static C++ normalization rules for note-body resource tag generation.

## Current Rules
- Prefers `resourcePath`-style package references and normalizes them through
  `WhatSon::Resources::normalizePath(...)`.
- Normalizes type/format through the same resource metadata helpers that back `.wsresource` packages.
- Emits canonical self-closing RAW tags with XML-escaped quoted attributes so drag/drop and clipboard import share one
  serialization rule.
