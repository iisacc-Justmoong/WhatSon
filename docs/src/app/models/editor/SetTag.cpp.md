# `src/app/models/editor/SetTag.cpp`

## Responsibility

Implements the `SetTag` editor-domain input object.

## Current Rules

- Static tag insertion is allow-list based; arbitrary XML tag names are rejected.
- Supported templates include transparent/editor semantic wrappers such as `callout`, `agenda`, `task`, `event`,
  `title`, inline formatting tags such as `bold` and `highlight`, canonical body `tag`, and the source divider token
  `break`.
- Selection mutation wraps selected source text between the opening and closing tokens.
- Empty selection mutation places the returned cursor position inside the inserted paired tag, or after the token for
  self-contained source tokens such as `break`.
- Document mutation always reuses `WhatSon::NoteBodyPersistence` for `.wsnbody` source projection and serialization
  so the editor input object does not own parser or persistence policy.

## Build Contract

- Registered by the recursive editor model CMake shard.
- Covered by `test/cpp/suites/editor_set_tag_tests.cpp` through the C++ regression target.
