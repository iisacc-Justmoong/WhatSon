# `src/app/models/editor/tags/ContentsEditorTagMutationBuilder.hpp`

## Responsibility

Declares the shortcut-independent RAW tag mutation payload builder.

## Current Contract

- The builder is a C++ `WhatSon.App.Internal` QML type.
- Callers provide source text, RAW selection offsets, and a canonical or alias tag name. They do not provide shortcut
  state.
- `normalizedTagName(...)` canonicalizes known editor tags including inline formatting tags, `callout`, `agenda`,
  `task`, `resource`, and `break`.
- `buildTagInsertionPayload(...)` is the preferred tag-generation entry point. It emits paired formatting/body wraps for
  selected ranges and canonical generated snippets only for collapsed break insertion.
- `buildWrappedTagInsertionPayload(...)` wraps the selected source range with a paired tag such as
  `<bold>...</bold>`, `<callout>...</callout>`, or `<agenda><task>...</task></agenda>`, or inserts an empty paired tag
  at a collapsed cursor.
- `tagInsertionPayloadBuilt(...)` is emitted after a payload is generated so existing command surfaces can observe tag
  generation without owning the mutation algorithm.

## Boundary

The builder owns tag-generation policy only. Shortcut mapping remains in `ContentsEditorTagInsertionController`, and
the editor session remains responsible for committing the returned RAW `.wsnbody` snapshot.

## 한국어

이 문서는 shortcut-independent 태그 생성 객체의 계약을 고정한다. 메뉴, 툴바, command palette 같은 비단축키
진입점은 canonical tag 이름을 결정한 뒤 이 builder의 payload API를 직접 호출해야 한다.
