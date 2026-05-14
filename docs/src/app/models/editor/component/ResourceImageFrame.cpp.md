# `src/app/models/editor/component/ResourceImageFrame.cpp`

## Responsibility

Implements the editor image resource-frame HTML renderer.

## Design Source

- Figma file: `fQUfzeMDED9JWvh4saYiVT`
- Node: `292:50`
- Shape: editor-width image resource frame with a 12px radius, `#2C2E2F` stroke, top `resourceHeader`,
  an auto-height media area, and bottom `resourceToolbar`.
- The emitted HTML intentionally exposes one `<img class="whatson-resource-frame">` object. The Figma chrome is painted
  into a cached PNG preview so Qt rich text treats the whole resource frame as one object replacement character.
- The frame is not wrapped in a synthetic outer block, because that block can confuse Qt rich-text flow height and let
  following note text intrude into the frame.
- The top and bottom text surfaces are display fields, not editor inputs: the top displays the resource type label and
  the bottom displays the resource file name.

## Runtime Behavior

- Image resources render at `width:100%` and `height:auto`, letting the decoded image size drive the visible height.
- `imageDisplaySize(...)` preserves the source image ratio until the derived height would exceed the width; taller
  images are capped to a square 1:1 display box. The emitted HTML carries `data-max-width-height-ratio="1:1"` and
  source/display dimensions so the editor session can preserve that contract across rich-text round-trips.
- The preview bitmap is generated in the application cache under `resource-frames/` and uses a bounded raster width
  while preserving the source image ratio. The editor HTML carries the source-derived display metadata, but the actual
  editable surface remains a single image object.
- The HTML keeps an `object-fit:contain` style marker for the design contract even though Qt rich text support for CSS
  object fitting is limited.
- The painted top header displays the normalized resource type label such as `Image`.
- The painted toolbar displays the note resource reference's file name, for example `capture.wsresource`.
- The type and filename are also exposed as `data-resource-type-label` and `data-resource-file-name` attributes for
  tests and diagnostics. They are display metadata, not editable text fields.
- The frame is surrounded by `<!--whatson-resource-source:...-->` comments so
  `WhatSonNoteBodyPersistence` can restore the exact canonical source tag when those comments survive the editor
  round-trip.
- If the user deletes the single image object but Qt leaves an empty marker pair behind, persistence treats that as a
  deleted resource frame and does not restore the canonical `<resource ... />` tag.
- `renderedTextLines(...)` remains as a legacy cleanup contract for older table-rendered frame residue that may still
  be present in synced editor text.

## 한국어

- 이 구현은 Figma `292:50`의 이미지 resource frame 구조를 Qt rich text HTML로 변환한다. 실제 HTML에는
  `whatson-resource-frame` class를 가진 `<img>` 하나만 노출하고, frame chrome은 캐시 PNG로 그린다.
- resource frame은 synthetic outer block을 사용하지 않는다. 별도 outer block은 Qt rich text 흐름 높이를
  흐트러뜨릴 수 있으므로 사용하지 않는다.
- 상단과 하단의 텍스트는 입력란이 아니라 표시란이다. 상단은 resource type, 하단은 resource file name을 표시한다.
- 이미지 영역은 에디터 폭을 채우고 `height:auto`로 렌더한다. 높이는 이미지 source 해상도의 비율을 따르되,
  높이가 폭을 넘는 경우에는 1:1 display box로 제한한다.
- editor가 보는 삭제 단위는 단일 image object다. 따라서 backspace/delete는 frame chrome의 개별 텍스트가 아니라
  resource frame 전체를 하나의 object replacement로 지운다.
- 실제 resource import, `.wsresource` 생성, source tag 삽입은 이 파일의 책임이 아니며, 이 파일은 표시 frame과
  저장 복원용 렌더 텍스트 계약만 소유한다.
