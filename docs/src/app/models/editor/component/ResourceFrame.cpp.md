# `src/app/models/editor/component/ResourceFrame.cpp`

## Responsibility

Implements the editor resource-frame HTML renderer.

## Design Source

- Figma file: `fQUfzeMDED9JWvh4saYiVT`
- Node: `292:50`
- Shape: 480px wide image resource frame with a 12px radius, `#2C2E2F` stroke, top `resourceHeader`,
  centered image preview viewport, and bottom `resourceToolbar`.
- The emitted HTML intentionally stays on a single-column table. This keeps the Figma `292:50` chrome recognizable
  while avoiding `colspan` and nested layout constructs that Qt `TextEdit.RichText` may serialize unpredictably.

## Runtime Behavior

- Image resources render inside a 338x352 preview viewport. The image is scaled to fit that viewport while preserving
  aspect ratio so Qt rich text can render it without a separate clipping layer.
- The top header displays the normalized resource type label such as `Image`.
- The toolbar displays the note resource reference's file name, for example `capture.wsresource`.
- The frame remains wrapped in `<!--whatson-resource-source:...-->` comments so
  `WhatSonNoteBodyPersistence` can restore the exact canonical source tag when those comments survive the editor
  round-trip.
- When Qt strips comments and serializes table text separately, `renderedTextLines(...)` provides the header,
  menu-dot, combined header/menu text, filename, and legacy label lines that `NoteEditorDocumentSession` must ignore
  while restoring the active source resource line.

## 한국어

- 이 구현은 Figma `292:50`의 이미지 resource frame 구조를 Qt rich text HTML로 변환한다. 실제 HTML은
  `TextEdit.RichText`의 저장 왕복을 안정화하기 위해 단일 컬럼 table을 사용한다.
- 실제 resource import, `.wsresource` 생성, source tag 삽입은 이 파일의 책임이 아니며, 이 파일은 표시 frame과
  저장 복원용 렌더 텍스트 계약만 소유한다.
