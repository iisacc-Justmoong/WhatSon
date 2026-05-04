# `src/app/models/editor/tags/ContentsEditorTagInsertionController.hpp`

## Responsibility

Declares the editor's common RAW tag insertion controller.

## Current Contract

- The controller is a C++ `WhatSon.App.Internal` QML type.
- Formatting commands and structured body commands share the same conceptual model: build a RAW `.wsnbody` tag
  insertion payload first, then let the normal editor session persist the resulting source snapshot.
- `tagNameForShortcutKey(...)` maps supported platform formatting shortcut keys to canonical tag names.
- `tagNameForBodyShortcutKey(...)` maps explicit tag-management shortcut keys to canonical body tag names such as
  `callout`, `agenda`, and `break`.
- `buildTagInsertionPayload(...)` is the preferred entry point. It emits paired formatting/body wraps for selected
  ranges and canonical body-tag snippets for collapsed cursor insertions.
- `buildWrappedTagInsertionPayload(...)` wraps the selected source range with a paired tag such as
  `<bold>...</bold>` or inserts an empty paired tag at a collapsed cursor. Generated body tags still use the canonical
  body snippet path at collapsed cursors.
- `normalizedTagName(...)` keeps canonical names for both inline formatting tags and body tags.

## Boundary

This controller only builds payloads. It does not render HTML, parse XML, or write note files.
