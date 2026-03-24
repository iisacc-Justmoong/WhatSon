# `src/app/file/note/WhatSonNoteFolderBindingService.hpp`

## Responsibility

This header declares the pure folder-binding rules for note headers. It keeps folder-array semantics
out of viewmodels and mutation coordinators.

## Public Contract

- `Bindings` is the normalized in-memory representation of `<folders>`.
- `bindings(...)` sanitizes and deduplicates raw folder/path UUID arrays.
- `mergeBindings(...)` preserves the primary source first, then appends missing secondary bindings.
- `assignFolder(...)` appends a dropped folder or rewrites the visible path for an existing UUID.
- `contains(...)` and `matches(...)` provide the canonical equality tests used by drag/drop and
  folder-tree mutations.

## Why It Exists

The application now allows a single note to belong to multiple folders. That means folder writes
must behave like array merges, not scalar replacement. This service centralizes that rule.
