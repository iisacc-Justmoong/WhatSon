# `src/app/models/editor/display`

## Responsibility

Contains editor-display support controllers and policies that are not QML view files.

## Current Modules

- `ContentsEditorSurfaceModeSupport.{hpp,cpp}`
  Decides whether `ContentViewLayout.qml` should mount the note editor surface or the direct resource editor surface.
- `ContentsEditorDisplayBackend.{hpp,cpp}`
  Owns the live note display session, projection, structured renderer, body-resource renderer, resource-tag
  controller, inline-resource presentation controller, and minimap metrics previously wired in QML. It does not own
  view-hook relay wiring; `ContentViewLayout.qml` dispatches view-local hooks directly.

## Boundary

XML parsing, HTML rendering, block-object construction, session synchronization, and deterministic geometry remain C++
responsibilities. Thin view wiring such as hook forwarding stays under `src/app/qml/view`.

## 한국어

이 섹션은 위 README 내용을 한국어로 확인하기 위한 하단 요약이다.

- 대상: ``src/app/models/editor/display`` (`docs/src/app/models/editor/display/README.md`)
- 위치: `docs/src/app/models/editor/display`
- 역할: 이 파일은 해당 디렉터리나 모듈의 구조, 책임, 운영 규칙, 검증 기준을 설명한다.
- 기준: 파일 경로, 명령, API 이름, 세부 변경 이력은 위 영어 본문을 원문 기준으로 유지한다.
- 변경 시: 위 영어 본문을 수정하면 이 한국어 하단 섹션도 함께 최신 상태로 맞춘다.
