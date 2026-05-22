# `src/app/models/editor/component/ResourceImageFrame.h`

## Responsibility

Declares the editor-side image resource frame renderer contract.

## Current Contract

- `ResourceFrameDescriptor` is the normalized data package consumed by the renderer. It contains the canonical
  `<resource ... />` source tag, note-body resource reference, resource id, type, format, resolved local asset path, and
  the current editor viewport width used for the media raster's intrinsic render width and dynamic centered placement.
  It may also carry `lockedFrameDisplayHeight`, the initial auto-height value recovered from a previous frame
  projection.
- `ResourceFrame` owns the display contract for standalone editor image resource lines:
  - source marker generation for `whatson-resource-source`
  - Figma node `292:50` image-frame chrome
  - 480px Figma design baseline metadata
  - marker-wrapped image-only atomic `<img>` frame rendering without a synthetic outer block or table row
  - neutral `<img>` CSS that avoids background/border character-format carryover into text typed after the image object
  - editor-width media raster rendering with initial auto height, optional height locking across viewport reprojection,
    dynamic centered image placement, and a square 1:1 maximum display box
  - no type, `...`, or filename display fields in the emitted frame HTML
- `NoteEditorDocumentSession` may parse and resolve a resource descriptor, but it must not hand-build the resource
  frame HTML.

## 한국어

- 이 헤더는 노트 에디터에 표시되는 image resource frame의 C++ 계약을 선언한다.
- standalone `<resource ... />` 라인의 atomic image frame, editor viewport 폭을 intrinsic width로 쓰는 responsive image
  media raster, frame 내부 이미지 표시 박스의 동적 중앙 정렬, Figma 480px design baseline metadata,
  이미지 source 비율 기반 초기 auto-height와 viewport 재투영 중 height lock, 1:1 최대 display box,
  synthetic outer block이나 table row 없는 image-only frame 렌더링을 `ResourceFrame`이 소유한다. `<img>` CSS에는
  배경/테두리를 싣지 않아 image object 뒤에 입력한 텍스트가 frame 스타일을 상속하지 않게 한다. type, `...`, file
  name 표시 필드나 진단 attribute는 frame HTML에 넣지 않는다.
