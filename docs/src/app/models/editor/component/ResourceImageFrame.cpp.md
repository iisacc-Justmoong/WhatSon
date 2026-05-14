# `src/app/models/editor/component/ResourceImageFrame.cpp`

## Responsibility

Implements the editor image resource-frame HTML renderer.

## Design Source

- Figma file: `fQUfzeMDED9JWvh4saYiVT`
- Node: `292:50`
- Shape: editor-width image resource frame with a 480px Figma design baseline, 12px radius, `#2C2E2F`
  stroke, 24px top `resourceHeader`, an auto-height media area, and 19px bottom `resourceToolbar`.
- Fixed Figma text metrics: Pretendard Regular 11px, 11px line height, 0 letter spacing, 8px horizontal
  padding, 4px vertical padding. The `more` affordance is a 16px icon made from three 2px `#CED0D6` dots.
- The emitted HTML exposes a structured `whatson-resource-frame` table. The frame chrome defines the layout first, then
  the top type label, the `...` affordance, the media image, and the bottom file-name label are emitted in that order.
- The header/footer labels and the `...` affordance are native editor text, not pixels painted into a raster preview.
  Only the media area uses a cached PNG so tall images can be capped to the 1:1 display box consistently.
- The frame is not wrapped in a synthetic outer block, because that block can confuse Qt rich-text flow height and let
  following note text intrude into the frame.
- The top and bottom text surfaces are display fields, not editor inputs: the top displays the resource type label and
  the bottom displays the resource file name.

## Runtime Behavior

- Image resources render at `width:100%` and `height:auto`, letting the decoded image size drive the visible height.
- `imageDisplaySize(...)` preserves the source image ratio until the derived height would exceed the width; taller
  images are capped to a square 1:1 display box. The emitted HTML carries `data-max-width-height-ratio="1:1"` and
  source/display dimensions so the editor session can preserve that contract across rich-text round-trips.
- The media bitmap is generated in the application cache under `resource-frames/` and uses the current editor
  viewport width supplied by `NoteEditorDocumentSession`, while still carrying the Figma 480px design baseline as
  metadata. This makes the media row fill the editor width in Qt rich text even where percentage image widths are not
  honored, without blurring the header/footer text.
- The cache key includes the resource-frame render version and fixed chrome metrics so old generated previews do not
  survive metric changes.
- The HTML keeps an `object-fit:contain` style marker for the design contract even though Qt rich text support for CSS
  object fitting is limited.
- The top header displays the normalized resource type label such as `Image` and the `...` affordance as text.
- The toolbar displays the note resource reference's file name, for example `capture.wsresource`, as text.
- The type and filename are also exposed as `data-resource-type-label` and `data-resource-file-name` attributes for
  tests and diagnostics. They are display metadata, not editable text fields.
- The frame is surrounded by `<!--whatson-resource-source:...-->` comments so
  `WhatSonNoteBodyPersistence` can restore the exact canonical source tag when those comments survive the editor
  round-trip.
- If the user deletes the media image object but Qt leaves an empty marker pair behind, persistence treats that as a
  deleted resource frame and does not restore the canonical `<resource ... />` tag.
- `renderedTextLines(...)` remains as a legacy cleanup contract for older table-rendered frame residue that may still
  be present in synced editor text.

## 한국어

- 이 구현은 Figma `292:50`의 이미지 resource frame 구조를 Qt rich text HTML로 변환한다. 실제 HTML에는
  `whatson-resource-frame` table을 노출하고, frame chrome이 먼저 레이아웃을 결정한다.
- 상단 resource type, 더보기 `...`, 하단 file name은 캐시 PNG 안에 그리지 않고 editor text로 둔다. 상단은
  24px, 하단은 19px이며, type/file name 텍스트는 Pretendard Regular 11px/line-height 11px 고정값을 사용한다.
- 이미지 영역만 현재 editor viewport 폭으로 캐시 PNG를 만들며, Figma 480px 기준은 design baseline metadata로만
  남긴다.
- resource frame은 synthetic outer block을 사용하지 않는다. 별도 outer block은 Qt rich text 흐름 높이를
  흐트러뜨릴 수 있으므로 사용하지 않는다.
- 상단과 하단의 텍스트는 입력란이 아니라 표시란이다. 상단은 resource type, 하단은 resource file name을 표시한다.
- 이미지 영역은 에디터 폭을 채우고 `height:auto`로 렌더한다. 높이는 이미지 source 해상도의 비율을 따르되,
  높이가 폭을 넘는 경우에는 1:1 display box로 제한한다.
- cached preview key는 frame render version과 고정 chrome metric을 포함하므로, 이전 렌더링 산출물이 새 metric
  변경을 가리지 않는다.
- editor plain text에는 frame text row와 image object가 함께 나타날 수 있다. 저장 경계는 image object가 제거된
  뒤 남는 type/file-name chrome 텍스트를 하나의 resource component residue로 보고 canonical source에 저장하지
  않는다.
- 실제 resource import, `.wsresource` 생성, source tag 삽입은 이 파일의 책임이 아니며, 이 파일은 표시 frame과
  저장 복원용 렌더 텍스트 계약만 소유한다.
