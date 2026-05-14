# `src/app/models/editor/component/ResourceImageFrame.h`

## Responsibility

Declares the editor-side image resource frame renderer contract.

## Current Contract

- `ResourceFrameDescriptor` is the normalized data package consumed by the renderer. It contains the canonical
  `<resource ... />` source tag, note-body resource reference, resource id, type, format, and resolved local asset path.
- `ResourceFrame` owns the display contract for standalone editor image resource blocks:
  - source marker generation for `whatson-resource-source`
  - Figma node `292:50` image-frame chrome
  - fixed Figma chrome metrics: 480px logical chrome width, 24px header, 19px toolbar, 11px label text, and 16px
    more icon with 2px dots
  - marker-wrapped single-image-object rendering without a synthetic outer block
  - editor-width image rendering with auto height and a square 1:1 maximum display box
  - display-only type and filename labels painted into the resource-frame preview
  - the legacy rendered text lines that persistence may ignore when old table chrome is serialized back to text
- `NoteEditorDocumentSession` may parse and resolve a resource descriptor, but it must not hand-build the resource
  frame HTML.

## 한국어

- 이 헤더는 노트 에디터에 표시되는 image resource frame의 C++ 계약을 선언한다.
- standalone `<resource ... />` 라인의 HTML frame, editor-width responsive frame, Figma 480px 논리 폭 기준의
  고정 chrome metric, 이미지 source 비율 기반 auto-height와 1:1 최대 display box, 입력란이 아닌 type/file name
  표시란, synthetic outer block 없는 단일 image object 렌더링, 이전 table chrome residue 저장 복원 시 건너뛸
  렌더 텍스트 목록은 `ResourceFrame`이 소유한다.
