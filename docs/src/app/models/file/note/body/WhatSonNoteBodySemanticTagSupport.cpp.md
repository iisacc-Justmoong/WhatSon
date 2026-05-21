# `src/app/models/file/note/body/WhatSonNoteBodySemanticTagSupport.cpp`

## Role
Implements the shared semantic-tag lookup used by note-body persistence and editor rendering.

## Key Behavior
- Normalizes tag-name checks through one case-folded path.
- Exposes canonical document-block lookup helpers so parser, renderer, and QML host code can agree on when one source
  tag should become its own document block.
- Treats `resource` and `break` as canonical note-body format blocks.
- Maps legacy title-like tags onto heading-style HTML spans using the same visual scale as the markdown heading preview:
  - `header` / `title` / `h1`
  - `subheader` / `subTitle` / `subtitle` / `h2`
  - `eventTitle` / `h3`
- The same registry now also distinguishes generic gap text (`type=text`) from explicit semantic text-tag blocks such
  as `paragraph`, `header`, `subheader`, `title`, `subtitle`, and `eventtitle`.
- Treats `next` as a rendered line-break alias without forcing the save path to drop that legacy source token.
- Treats `style` as a source-preserved wrapper whose attributes are interpreted by note-body persistence rather than
  escaped as literal XML.
- Treats transparent wrappers such as `event` and `callout` as source semantic containers whose children render without
  painting the wrapper tag literally.
- Keeps the source-projection block whitelist narrower than the rendered-semantic whitelist so legacy semantic tags can
  survive round-trips as raw source while still rendering as meaningful HTML.

## Regression Notes
- Adding a new `.wsnbody` semantic tag now requires updating this registry first; renderer/persistence code should
  consume the registry instead of adding new ad-hoc string comparisons.
