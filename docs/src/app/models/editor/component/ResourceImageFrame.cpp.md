# `src/app/models/editor/component/ResourceImageFrame.cpp`

## Responsibility

Implements the editor image resource-frame HTML renderer.

## Design Source

- Figma file: `fQUfzeMDED9JWvh4saYiVT`
- Node: `292:50`
- Shape: editor-width image resource frame with a 480px Figma design baseline, 12px radius, `#2C2E2F`
  stroke, and one auto-height media area.
- The emitted HTML exposes a `whatson-resource-frame` table as the component container. That container is still the
  resource frame model; the image is not emitted as an independent loose image.
- The frame renders only one visible child: the media `<img>`. Type, `...`, and file-name values are not emitted into the
  frame HTML as display text or diagnostic attributes.
- The frame is not wrapped in a synthetic outer block, because that block can confuse Qt rich-text flow height and let
  following note text intrude into the frame.
- There are no visible header/footer text surfaces in the current image-only frame contract.

## Runtime Behavior

- Image resources render at `width:100%` and `height:auto`, letting the decoded image size drive the visible height.
- `imageDisplaySize(...)` preserves the source image ratio until the derived height would exceed the width; taller
  images are capped to a square 1:1 display box. The emitted HTML carries `data-max-width-height-ratio="1:1"` and
  source/display dimensions so the editor session can preserve that contract across rich-text round-trips.
- The image bitmap is generated in the application cache under `resource-frames/` and uses the current editor
  viewport width supplied by `NoteEditorDocumentSession`, while still carrying the Figma 480px design baseline as
  metadata. This makes the image row fill the editor width in Qt rich text even where percentage image widths are not
  honored.
- The cache key includes the resource-frame render version so old generated previews do not survive contract changes.
- The HTML keeps an `object-fit:contain` style marker for the design contract even though Qt rich text support for CSS
  object fitting is limited.
- The frame is surrounded by `<!--whatson-resource-source:...-->` comments so
  `WhatSonNoteBodyPersistence` can restore the exact canonical source tag when those comments survive the editor
  round-trip.
- If the user deletes the media image object but Qt leaves an empty marker pair behind, persistence treats that as a
  deleted resource frame and does not restore the canonical `<resource ... />` tag.
## 한국어

- 이 구현은 Figma `292:50`의 이미지 resource frame 구조를 Qt rich text HTML로 변환한다. 실제 HTML에는
  `whatson-resource-frame` table을 노출하고, 이 table이 frame container 역할을 한다.
- frame 안에서 보이는 콘텐츠는 이미지 하나뿐이다. resource type, `...`, file name은 화면 텍스트나
  diagnostic attribute로도 내보내지 않는다.
- 이미지는 현재 editor viewport 폭으로 캐시 PNG를 만들며, Figma 480px 기준은 design baseline metadata로만 남긴다.
- resource frame은 synthetic outer block을 사용하지 않는다. 별도 outer block은 Qt rich text 흐름 높이를
  흐트러뜨릴 수 있으므로 사용하지 않는다.
- 이미지 영역은 에디터 폭을 채우고 `height:auto`로 렌더한다. 높이는 이미지 source 해상도의 비율을 따르되,
  높이가 폭을 넘는 경우에는 1:1 display box로 제한한다.
- cached preview key는 frame render version을 포함하므로, 이전 렌더링 산출물이 새 계약 변경을 가리지 않는다.
- editor plain text에는 image object만 나타나야 한다. 이전 table chrome 텍스트가 남아 들어오는 경우에는 legacy
  residue로 보고 canonical source에 저장하지 않는다.
- 실제 resource import, `.wsresource` 생성, source tag 삽입은 이 파일의 책임이 아니며, 이 파일은 표시 frame
  렌더링 계약만 소유한다.
