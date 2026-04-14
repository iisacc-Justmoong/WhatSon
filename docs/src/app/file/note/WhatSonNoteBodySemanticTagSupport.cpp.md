# `src/app/file/note/WhatSonNoteBodySemanticTagSupport.cpp`

## Role
Implements the shared semantic-tag lookup used by note-body persistence and editor rendering.

## Key Behavior
- Normalizes tag-name checks through one case-folded path.
- Maps legacy title-like tags onto heading-style HTML spans using the same visual scale as the markdown heading preview:
  - `title` / `h1`
  - `subTitle` / `subtitle` / `h2`
  - `eventTitle` / `h3`
- Treats `next` as a rendered line-break alias without forcing the save path to drop that legacy source token.
- Marks `event` as a transparent semantic container so read-side renderers can consume its children without painting the
  wrapper tag literally.
- Keeps the source-projection block whitelist narrower than the rendered-semantic whitelist so legacy semantic tags can
  survive round-trips as raw source while still rendering as meaningful HTML.

## Regression Notes
- Adding a new `.wsnbody` semantic tag now requires updating this registry first; renderer/persistence code should
  consume the registry instead of adding new ad-hoc string comparisons.
