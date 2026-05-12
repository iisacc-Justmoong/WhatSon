# `src/app/qml`

## Role
This directory contains the root LVRS application shell and QML-side interaction surfaces.

The core rule in this directory is that visual composition belongs here, while persistence and mutation logic stay in C++ model-domain controllers or narrow bridge/helper objects under `src/app/models`.

## Important Files
- `Main.qml`: the root `LV.ApplicationWindow`, route shell, and restored desktop/mobile workspace host.
- `DesignTokens.qml`: QML-side design token aggregation.
- `view/contents/Gutter.qml`, `view/contents/TextEditor.qml`, `view/contents/Minimap.qml`: the only contents-internal
  QML views allowed for the editor route. `ContentViewLayout.qml` mounts these three views inside the restored shell.

## LVRS Token Rule
- QML view files must express reusable colors, transparency, spacing, typography, and fixed UI extents through
  `LV.Theme` tokens or token compositions.
- Direct visual literals such as `#...`, `Qt.rgba(...)`, `color: "transparent"`, `spacing: 0`, and raw
  `font.pixelSize` values are reserved for non-visual data/algorithmic cases only.

## Ownership Model
The C++ composition root applies `WhatSonQmlContextBinder` before root QML load. `Main.qml` keeps startup/onboarding
routing and workspace chrome, and forwards only the narrow `NoteEditorDocumentSession` object into the TextEditor
surface. The editor document binding is the editor HTML session file path that reaches `LV.TextEditor.filePath`;
`.wsnbody` parse/source projection/serialize remains in C++. Focused editor format shortcuts live in
`ContentViewLayout.qml` only as command dispatch and call the C++ `NoteEditorDocumentSession` format API. Gutter cursor
indication follows the LVRS editor's rendered cursor rectangle and line metrics, not raw newline counting in the
RichText HTML document.

## Why This Directory Is Important
If a runtime object exists in C++ but behaves incorrectly in the UI, this directory is usually where the mismatch becomes visible first.

## 한국어

이 섹션은 위 README 내용을 한국어로 확인하기 위한 하단 요약이다.

- 대상: ``src/app/qml`` (`docs/src/app/qml/README.md`)
- 위치: `docs/src/app/qml`
- 역할: 이 파일은 해당 디렉터리나 모듈의 구조, 책임, 운영 규칙, 검증 기준을 설명한다.
- 현재 workspace 인터페이스는 기존 shell을 유지하고, content slot은 `ContentViewLayout.qml`을 통해 선택된
  노트의 editor HTML session file을 LVRS TextEditor에 연결한다. 포맷 단축키는 이 layout의 얇은 command
  dispatch로만 존재하고 실제 source/projection 처리는 C++ 세션에 맡긴다. 거터의 현재 줄 표시는 RichText HTML
  문자열의 줄바꿈 문자가 아니라 LVRS editor의 렌더 cursor rectangle과 line metric을 따른다.
- 기준: 파일 경로, 명령, API 이름, 세부 변경 이력은 위 영어 본문을 원문 기준으로 유지한다.
- 변경 시: 위 영어 본문을 수정하면 이 한국어 하단 섹션도 함께 최신 상태로 맞춘다.
