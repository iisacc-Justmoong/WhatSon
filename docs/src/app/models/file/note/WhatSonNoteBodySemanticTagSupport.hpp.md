# `src/app/models/file/note/WhatSonNoteBodySemanticTagSupport.hpp`

## Role
Declares the shared `.wsnbody` semantic-tag registry that keeps note-body save/load parsing and editor HTML rendering on
the same tag-classification rules.

## Shared Contracts
- Centralizes the canonical inline-style alias map (`bold`, `italic`, `underline`, `strikethrough`, `highlight`).
- Also exposes the canonical proprietary inline hyperlink tag name `weblink`.
- Distinguishes source-projection block tags from richer rendered-semantic block tags:
  - source projection keeps the legacy/editor-safe block whitelist
  - rendered semantics additionally recognize legacy body tags such as `title`, `subTitle`, `eventTitle`, and
    `eventDescription`
- Distinguishes source-only passthrough tags that must survive note-body serialization without being escaped:
  - `next`
  - `event`
  - `callout`
  - `title`
  - `subTitle` / `subtitle`
  - `eventTitle`
  - `eventDescription`
- Classifies body-format document blocks shared by persistence, parser, and renderer:
  - `resource`
  - `break` / `hr`
- Classifies transparent paired wrappers shared by persistence and read-side renderers:
  - `agenda` / `task`
  - `callout`
- Exposes semantic HTML open/close helpers so read-side HTML projections do not hardcode their own heading styling.

## Why It Exists
Before this helper, `WhatSonNoteBodyPersistence` and `ContentsTextFormatRenderer` maintained separate tag whitelists.
That made the same stored `.wsnbody` element render as styled content in one path and literal text in another.
