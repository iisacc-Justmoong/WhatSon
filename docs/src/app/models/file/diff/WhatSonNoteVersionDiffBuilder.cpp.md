# `src/app/models/file/diff/WhatSonNoteVersionDiffBuilder.cpp`

## Role
Implements diff segment construction and segment application for note version payloads.

## Behavior
- Computes common prefix and suffix lengths between the previous and next text payloads.
- Stores the removed and inserted text slices in the diff segment.
- Emits a git-like unified patch using the caller-provided label, such as `header.wsnhead` or `body.wsnbody`.
- Records `generatedAtUtc` with the current UTC time when the diff segment is built.
- `applyDiffSegmentOntoCurrent(...)` validates that the segment matches its base, then applies only the removed/inserted
  slice onto the current text using nearby prefix/suffix anchors.
- Whole-document replacement diffs are refused when the current text has already diverged from the base; callers must
  handle that as a conflict instead of silently replacing the user's current document.

## Boundary
- This file is deliberately stateless.
- Snapshot lookup, version JSON serialization, and UTF-8 file IO belong to sibling collaborators.
