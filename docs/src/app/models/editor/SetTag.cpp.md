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
- The public `bold` command now authors `<style weight="900">...</style>` instead of a new `<bold>...</bold>` wrapper.
  Legacy `<bold>` source is still read by persistence, but new editor commands bind bold to the same style `weight`
  axis used by the toolbar so `<bold>` and `<style>` do not overlap.
- `insertStyleTagIntoSource(...)` is the specialized style wrapper entrypoint used by the editor toolbar. It validates
  the requested style value through `component/style`, preserves toggle behavior through the shared insertion flow, and
  emits the bare `<style>` token for Body fallback.
- `insertStyleFontTagIntoSource(...)` is the matching font-family entrypoint for the editor toolbar. It validates and
  escapes the family through `component/style`, preserves the shared insertion flow, and returns `fontFamily` in the
  result map.
- `insertStyleFontSizeTagIntoSource(...)` is the matching font-size entrypoint for the editor toolbar. It validates the
  positive integer size through `component/style`, preserves the shared insertion flow, and returns `fontSize` in the
  result map.
- `insertStyleFontWeightTagIntoSource(...)` is the matching weight entrypoint. The `bold` command reuses this style
  weight token shape with the normalized `900` value.
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
