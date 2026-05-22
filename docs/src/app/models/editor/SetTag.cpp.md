# `src/app/models/editor/SetTag.cpp`

## Responsibility

Implements the `SetTag` editor-domain input object.

## Current Rules

- Static tag insertion is allow-list based; arbitrary XML tag names are rejected.
  heading-style wrappers such as `header`, `subheader`, and `title`, inline formatting tags such as `bold` and
  `highlight`, canonical body `tag`, the custom `<style>...</style>` wrapper, the source divider token `break`, and a
  placeholder `resource` body block.
  `component/style` provides the style wrapper tokens while `SetTag` converts that descriptor into the generic insertion
  flow.
- Selection mutation wraps selected source text between the opening and closing tokens.
- If a non-empty selection is already exactly enclosed by the same paired static tag, applying that tag toggles the
  format off by removing the surrounding tokens instead of nesting a duplicate wrapper. The result reports this through
  `toggledOff`.
- Empty selection mutation places the returned cursor position inside the inserted paired tag, or after the token for
  self-contained source tokens such as `break`.
- Self-contained document-body tokens such as `break` and `resource` are inserted on a standalone source line so
  `WhatSon::NoteBodyPersistence` can serialize them as direct `.wsnbody <body>` children.
- Document mutation always reuses `WhatSon::NoteBodyPersistence` for `.wsnbody` source projection and serialization
  so the editor input object does not own parser or persistence policy.

## Build Contract

- Registered by the recursive editor model CMake shard.
- Covered by `test/cpp/suites/editor_set_tag_tests.cpp` through the C++ regression target.
