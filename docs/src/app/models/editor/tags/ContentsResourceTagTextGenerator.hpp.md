# `src/app/models/editor/tags/ContentsResourceTagTextGenerator.hpp`

## Responsibility
Declares the C++ bridge that builds canonical RAW `<resource ... />` tag text for editor imports.

## Boundary
- Belongs to `editor/tags` because it creates editor-body source markup, not resource package storage.
- Low-level descriptor normalization remains delegated to `WhatSonNoteBodyResourceTagGenerator`.
