# `src/app/models/editor/component/ResourceImageFrame.h`

## Responsibility

Declares the editor-side image resource frame renderer contract.

## Current Contract

- `ResourceFrameDescriptor` is the normalized data package consumed by the renderer. It contains the canonical
  `<resource ... />` source tag, note-body resource reference, resource id, type, format, resolved local asset path, and
  the current editor viewport width used for the media image's intrinsic render width.
- `ResourceFrame` owns the display contract for standalone editor image resource blocks:
  - source marker generation for `whatson-resource-source`
  - Figma node `292:50` image-frame chrome
  - fixed Figma chrome metrics: 480px design baseline, 24px header, 19px toolbar, 11px label text, and 16px more icon
    with 2px dots
  - marker-wrapped structured-frame rendering without a synthetic outer block
  - editor-width image rendering with auto height and a square 1:1 maximum display box
  - display-only type, `...`, and filename labels emitted as native editor text
  - the legacy rendered text lines that persistence may ignore when old table chrome is serialized back to text
- `NoteEditorDocumentSession` may parse and resolve a resource descriptor, but it must not hand-build the resource
  frame HTML.

## 한국어

- 이 헤더는 노트 에디터에 표시되는 image resource frame의 C++ 계약을 선언한다.
- standalone `<resource ... />` 라인의 HTML frame, editor viewport 폭을 intrinsic width로 쓰는 responsive image
  media, Figma 480px design baseline 기준의 고정 chrome metric, 이미지 source 비율 기반 auto-height와 1:1 최대
  display box, 입력란이 아닌 type/`...`/file name 표시 텍스트, synthetic outer block 없는 structured frame 렌더링,
  이전 table chrome residue 저장 복원 시 건너뛸 렌더 텍스트 목록은 `ResourceFrame`이 소유한다.
