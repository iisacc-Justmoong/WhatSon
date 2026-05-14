# `src/app/models/editor/component/ResourceFrame.h`

## Responsibility

Declares the editor-side resource frame renderer contract.

## Current Contract

- `ResourceFrameDescriptor` is the normalized data package consumed by the renderer. It contains the canonical
  `<resource ... />` source tag, note-body resource reference, resource id, type, format, and resolved local asset path.
- `ResourceFrame` owns the display contract for standalone editor resource blocks:
  - source marker generation for `whatson-resource-source`
  - Figma node `292:50` image-frame chrome
  - the 338x352 image preview viewport
  - the rendered text lines that persistence must ignore when Qt serializes the rich frame back to text
- `NoteEditorDocumentSession` may parse and resolve a resource descriptor, but it must not hand-build the resource
  frame HTML.

## 한국어

- 이 헤더는 노트 에디터에 표시되는 resource frame의 C++ 계약을 선언한다.
- standalone `<resource ... />` 라인의 HTML frame, Figma 기준 preview viewport, 저장 복원 시 건너뛸 렌더 텍스트
  목록은 `ResourceFrame`이 소유한다.
