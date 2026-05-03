# `src/app/models/editor/session`

## Responsibility
Owns the C++ editor session controller.

## Current Modules
- `ContentsEditorSessionController.*`
  C++ session authority for pending saves, same-note echo acceptance, synchronized editor text state, and accepted local
  RAW source commits.

## Boundary
- Editor hosts must instantiate `ContentsEditorSessionController` directly through the internal C++ QML type.
- This directory must not contain QML wrappers. QML is reserved for view construction and must not carry editor
  session, persistence, or synchronization policy.
- Local editor mutations must enter through `commitRawEditorTextMutation(...)`; QML controllers may build candidate RAW
  text, but they must not write `editorText`, mark local authority, or schedule persistence themselves.
- Explicit tag-management mutations may follow that session commit with
  `persistEditorTextImmediatelyWithText(...)` so discrete RAW tag insertions are flushed through the same persistence
  boundary without waiting for the idle typing path.
- Persistence decisions must still route through RAW `.wsnote/.wsnbody` mutation and sync paths.

## 한국어

이 섹션은 위 README 내용을 한국어로 확인하기 위한 하단 요약이다.

- 대상: ``src/app/models/editor/session`` (`docs/src/app/models/editor/session/README.md`)
- 위치: `docs/src/app/models/editor/session`
- 역할: 이 파일은 해당 디렉터리나 모듈의 구조, 책임, 운영 규칙, 검증 기준을 설명한다.
- 기준: 파일 경로, 명령, API 이름, 세부 변경 이력은 위 영어 본문을 원문 기준으로 유지한다.
- 변경 시: 위 영어 본문을 수정하면 이 한국어 하단 섹션도 함께 최신 상태로 맞춘다.
