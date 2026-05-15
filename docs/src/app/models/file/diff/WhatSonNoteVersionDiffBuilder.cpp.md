# `src/app/models/file/diff/WhatSonNoteVersionDiffBuilder.cpp`

## Role
Implements diff segment construction for note version payloads.

## Behavior
- Computes common prefix and suffix lengths between the previous and next text payloads.
- Stores the removed and inserted text slices in the diff segment.
- Emits a git-like unified patch using the caller-provided label, such as `header.wsnhead` or `body.wsnbody`.

## Boundary
- This file is deliberately stateless.
- Snapshot lookup, version JSON serialization, and UTF-8 file IO belong to sibling collaborators.
