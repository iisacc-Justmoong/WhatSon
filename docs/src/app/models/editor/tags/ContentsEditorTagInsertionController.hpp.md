# `src/app/models/editor/tags/ContentsEditorTagInsertionController.hpp`

## Responsibility

Declares the editor's shortcut-facing tag insertion controller facade.

## Current Contract

- The controller is a C++ `WhatSon.App.Internal` QML type.
- `tagNameForShortcutKey(...)` maps supported platform formatting shortcut keys to canonical tag names.
- `tagNameForBodyShortcutKey(...)` maps explicit tag-management shortcut keys to canonical body tag names such as
  `callout`, `agenda`, and `break`.
- `mutationBuilder` exposes the shortcut-independent `ContentsEditorTagMutationBuilder` owned by this facade.
- `buildTagInsertionPayload(...)`, `buildWrappedTagInsertionPayload(...)`, and `normalizedTagName(...)` are
  compatibility delegations to `ContentsEditorTagMutationBuilder`; new UI entry points should call the builder after
  they have chosen a canonical tag name.

## Boundary

This controller resolves shortcut keys and delegates payload generation. It does not render HTML, parse XML, own RAW
tag-generation policy, or write note files.
