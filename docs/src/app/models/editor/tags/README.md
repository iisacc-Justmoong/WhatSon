# `src/app/models/editor/tags`

## Responsibility
Owns editor-body tag-management helpers that are not inline text formatting.

## Non-format editor body tags
- `<agenda>` / `<task>` parsing, insertion payloads, task toggles, and agenda Enter behavior.
- `<callout>` parsing, insertion payloads, and callout Enter behavior.
- `<break>` canonicalization and structured verification.
- `<resource ... />` RAW source insertion and canonical resource-tag text generation.
- Structured body-tag linting and advisory correction state for parser/renderer feedback.

## Current Modules
- `ContentsAgendaBackend.*`
- `ContentsCalloutBackend.*`
- `ContentsStructuredTagValidator.*`
- `WhatSonStructuredTagLinter.*`
- `ContentsResourceTagTextGenerator.*`
- `ContentsResourceTagController.qml`

## Boundary
- Inline style tags such as bold, italic, highlight, and links remain under editor formatting responsibilities.
- Workspace hierarchy tag storage remains under `src/app/models/file/hierarchy/tags`.
- General hub/package/storage validators remain under `src/app/models/file/validator`.
- Resource import orchestration remains under `src/app/models/editor/resource`; only RAW `<resource ... />` tag
  construction and insertion policy live here.
