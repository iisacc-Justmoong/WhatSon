# `src/app/models/editor/SetTag.h`

## Responsibility

Declares the `SetTag` editor-domain input object.

`SetTag` owns the fixed allow-list and generic mutation flow for `.wsnbody` RAW tag templates, while component-specific
methods for inserting those templates into editor source text or into a serialized body document.

## Public Contract

- Default tag: `callout`.
- Known static templates include paired wrappers such as `header`, `subheader`, `style`, and standalone placeholders such
  as `resource`.
- `configureTagName(...)` changes the active tag only when the requested name resolves to a known static template.
- `insertIntoSource(...)` inserts the active template into editor-facing body source text.
- `insertNamedTagIntoSource(...)` performs the same mutation without changing the active tag.
- `insertStyleTagIntoSource(...)` inserts the custom style wrapper with a canonical `style` attribute value. The
  `Body` value intentionally serializes as `<style>...</style>` because missing/empty style attributes fall back to
  Body.
- `insertStyleFontTagIntoSource(...)` inserts the custom style wrapper with an explicit `font` attribute value, for
  example `<style font="Menlo">...</style>`.
- `insertStyleFontSizeTagIntoSource(...)` inserts the custom style wrapper with an explicit `size` attribute value, for
  example `<style size="18">...</style>`.
- `insertStyleFontWeightTagIntoSource(...)` inserts the custom style wrapper with an explicit `weight` attribute value.
  The editor `bold` command uses this shape and emits `<style weight="900">...</style>` instead of new `<bold>` source.
- `insertStyleBackgroundTagIntoSource(...)` inserts the custom style wrapper with an explicit `background` attribute
  value, validated by the style component against the bookmark color palette/hex contract.
- Paired inline/static wrappers are toggle-aware: when the selected source text is exactly enclosed by the same tag,
  mutation removes that wrapper and returns `toggledOff: true`.
- `insertIntoBodyDocument(...)` and `insertNamedTagIntoBodyDocument(...)` project a `.wsnbody` document back to
  editor source, apply the static source mutation, and reserialize the document through
  `WhatSon::NoteBodyPersistence::serializeBodyDocument(...)`.
- `resource` is a static placeholder block (`<resource />`) and is line-isolated during source mutation so persistence
  stores it as a direct body child rather than escaped paragraph prose.
- `style` is a paired static wrapper (`<style>...</style>`) whose wrapper tokens, canonical `style` attribute values,
  and projection helpers live in `component/style`. Attribute values such as `font`, `size`, `color`, `background`, and
  existing `height` are set by the normal tag-attribute path and interpreted by the style component during HTML
  projection.
- Unsupported tag names return an invalid result and leave source/document text unchanged.

## Signals And Slots

- Signals: `tagNameChanged`, `lastErrorChanged`, `tagInserted`
- Slots: `setTagName(...)`, `clearLastError()`

## Build Contract

- Registered by `src/app/models/editor/CMakeLists.txt`.
- Kept under `src/app/models/editor` so editor-domain model expansion does not require direct source entries in the
  root app CMake file.
