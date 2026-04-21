# `src/app/models/file/note/WhatSonNoteBodyResourceTagGenerator.hpp`

## Role
Declares the canonical RAW `<resource ... />` tag builder used by note-body import flows.

## Exposed API
- `normalizeImportedResourceDescriptor(...)`: converts import metadata into a stable `{type, format, path, id}`
  descriptor.
- `buildCanonicalResourceTag(...)`: emits one quoted, self-closing `<resource ... />` tag in canonical attribute order.
