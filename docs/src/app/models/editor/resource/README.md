# `src/app/models/editor/resource`

## Responsibility
Owns editor-side resource import and inline resource presentation coordination.

## Current Modules
- `ContentsResourceImportController.*`
  Public import coordinator mounted by the editor host.
- `ContentsResourceDropPayloadParser.*`
  Normalizes drag/drop payloads into importable resource URLs.
- `ContentsResourceImportConflictController.*`
  Owns duplicate-resource prompt state.
- `ContentsInlineResourcePresentationController.*`
  Builds rounded Figma `292:50` frame image projections and structured `resourceVisualBlocks` used by note-body
  resource delegates. Inline resource frames resolve 100% width to the current editor text column before the cached
  frame image and matching visual block dimensions are generated. Legacy HTML placeholder replacement remains in the
  lower-level presenter for compatibility, but the live editor surface consumes visual block records directly.
- `ContentsEditorSurfaceGuardController.*`
  Guards programmatic editor-surface sync during resource import turns.

## Boundary
- RAW `<resource ... />` tag construction and insertion helpers live under `src/app/models/editor/tags`.
- Visual resource cards and viewers remain under `src/app/qml/view/contents/editor`.
- File/resource storage and bitmap/PDF rendering backends remain under `src/app/models/file`.

## 한국어

이 섹션은 위 README 내용을 한국어로 확인하기 위한 하단 요약이다.

- 대상: ``src/app/models/editor/resource`` (`docs/src/app/models/editor/resource/README.md`)
- 위치: `docs/src/app/models/editor/resource`
- 역할: 이 파일은 해당 디렉터리나 모듈의 구조, 책임, 운영 규칙, 검증 기준을 설명한다.
- 기준: 파일 경로, 명령, API 이름, 세부 변경 이력은 위 영어 본문을 원문 기준으로 유지한다.
- 변경 시: 위 영어 본문을 수정하면 이 한국어 하단 섹션도 함께 최신 상태로 맞춘다.
