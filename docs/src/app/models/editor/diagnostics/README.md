# `src/app/models/editor/diagnostics`

## Responsibility
Owns editor-domain diagnostic support that can be shared by view hosts, controllers, and model-facing QML helpers.

## Current Modules
- `ContentsEditorDebugTrace.js`
  Formats compact debug trace strings for editor QML objects without leaving diagnostics utilities inside the view
  directory.

## Boundary
- Diagnostics helpers must not own editor state or mutation policy.
- View QML may import diagnostics helpers, but visual layout remains under `src/app/qml/view/content/editor`.

## 한국어

이 섹션은 위 README 내용을 한국어로 확인하기 위한 하단 요약이다.

- 대상: ``src/app/models/editor/diagnostics`` (`docs/src/app/models/editor/diagnostics/README.md`)
- 위치: `docs/src/app/models/editor/diagnostics`
- 역할: 이 파일은 해당 디렉터리나 모듈의 구조, 책임, 운영 규칙, 검증 기준을 설명한다.
- 기준: 파일 경로, 명령, API 이름, 세부 변경 이력은 위 영어 본문을 원문 기준으로 유지한다.
- 변경 시: 위 영어 본문을 수정하면 이 한국어 하단 섹션도 함께 최신 상태로 맞춘다.
