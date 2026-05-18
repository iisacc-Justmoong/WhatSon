# `src/app/models/editor/SetTag.h`

## Responsibility

Declares the `SetTag` editor-domain input object.

`SetTag` owns the fixed allow-list and generic mutation flow for `.wsnbody` RAW tag templates, while component-specific
template definitions such as Agenda live in `src/app/models/editor/component`. It exposes QML/Qt-friendly mutation
methods for inserting those templates into editor source text or into a serialized body document.

## Public Contract

- Default tag: `callout`
- `availableTagNames()` returns the static body-tag names that can be inserted, including `agenda`, `task`, `header`,
  `subheader`, and `resource`.
- `configureTagName(...)` changes the active tag only when the requested name resolves to a known static template.
- `insertIntoSource(...)` inserts the active template into editor-facing body source text.
- `insertNamedTagIntoSource(...)` performs the same mutation without changing the active tag.
- Paired inline/static wrappers are toggle-aware: when the selected source text is exactly enclosed by the same tag,
  mutation removes that wrapper and returns `toggledOff: true`.
- `insertIntoBodyDocument(...)` and `insertNamedTagIntoBodyDocument(...)` project a `.wsnbody` document back to
  editor source, apply the static source mutation, and reserialize the document through
  `WhatSon::NoteBodyPersistence::serializeBodyDocument(...)`.
- `resource` is a static placeholder block (`<resource />`) and is line-isolated during source mutation so persistence
  stores it as a direct body child rather than escaped paragraph prose.
- Unsupported tag names return an invalid result and leave source/document text unchanged.

## Signals And Slots

- Signals: `tagNameChanged`, `lastErrorChanged`, `tagInserted`
- Slots: `setTagName(...)`, `clearLastError()`

## Build Contract

- Registered by `src/app/models/editor/CMakeLists.txt`.
- Kept under `src/app/models/editor` so editor-domain model expansion does not require direct source entries in the
  root app CMake file.
