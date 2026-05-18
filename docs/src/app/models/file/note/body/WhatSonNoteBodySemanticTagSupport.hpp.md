# `src/app/models/file/note/body/WhatSonNoteBodySemanticTagSupport.hpp`

## Role
Declares the shared `.wsnbody` semantic-tag registry used by note-body save/load parsing.

## Shared Contracts
- Centralizes the canonical inline-style alias map (`bold`, `italic`, `underline`, `strikethrough`, `highlight`).
- Also exposes the canonical proprietary inline hyperlink tag name `weblink`.
- Distinguishes body-source block tags from passthrough semantic tags such as `header`, `subheader`, `title`,
  `subTitle`, `eventTitle`, and `eventDescription`.
- Distinguishes source-only passthrough tags that must survive note-body serialization without being escaped:
  - `next`
  - `event`
  - `callout`
  - `header`
  - `subheader`
  - `title`
  - `subTitle` / `subtitle`
  - `eventTitle`
  - `eventDescription`
- Classifies body-format document blocks shared by persistence:
  - `resource`
  - `break` / `hr`
- Classifies transparent paired wrappers shared by persistence:
  - `callout`
- Exposes semantic HTML open/close helpers for non-editor read-side consumers.

## Why It Exists
Before this helper, note-body persistence had multiple local tag whitelists. This helper keeps source serialization
classification in one place.
