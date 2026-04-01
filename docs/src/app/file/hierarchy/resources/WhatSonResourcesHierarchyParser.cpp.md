# `src/app/file/hierarchy/resources/WhatSonResourcesHierarchyParser.cpp`

## Responsibility

Reads `Resources.wsresources` payloads and restores a normalized `.wsresource` package-path list.

## Accepted Input Forms

- Legacy JSON string arrays.
- Object root with `resources: [...]`.
- Object array entries with `resourcePath` or legacy `path` (case-insensitive key match).
- Root object direct path key (`resourcePath` / `path`).
- XML-like tag text such as `<resource ... path=...>` or `<resource ... resourcePath=...>`.
- Final fallback: line-based plain text.

Attribute values inside `<resource ...>` tags are parsed in all supported forms:

- Double-quoted (`path="..."`).
- Single-quoted (`path='...'`).
- Bare values (`resourcePath=...`).

The parser also accepts mixed casing for attribute keys (for example `PATH`).

## Compatibility

Writer output now prefers object-array format, but parser compatibility remains backward-safe for
legacy hubs, so immediate migration is not required for runtime loading.

`<resources>`-style wrapper lines are ignored during fallback line parsing and are not treated as
resource paths.

## Regex Literal Safety Note

Resource-tag attribute regex literals use escaped `QStringLiteral(...)` strings instead of C++ raw
string literals to avoid accidental raw-literal terminator collisions in patterns containing
attribute quotes.
