# `src/app/qml/contents`

## Role
This directory hosts standalone contents-surface view artifacts that mirror Figma content frames before they are wired
into the runtime editor stack.

## Current Files
- `ContentsView.qml`: Figma node `155:4561` root frame and LVRS layout host.
- `Gutter.qml`: line-number rail and change/conflict markers for Figma node `155:5345`.
- `EditorView.qml`: read-only text projection for Figma node `155:5352`.
- `Minimap.qml`: minimap row rail for Figma node `352:8626`.

## Boundary
- The view is presentation-only.
- It does not own note persistence, parser state, or editor mutation authority.
- Every QML file imports LVRS and expresses colors, spacing, typography, and fixed rails through `LV.Theme` tokens.
- Figma dimensions are preserved through token compositions instead of literal pixel values.

## Tests
- `test/cpp/suites/qml_contents_view_tests.cpp` locks the four-file composition contract and the read-only editor
  projection boundary.

## 한국어

이 섹션은 위 README 내용을 한국어로 확인하기 위한 하단 요약이다.

- 대상: ``src/app/qml/contents`` (`docs/src/app/qml/contents/README.md`)
- 위치: `docs/src/app/qml/contents`
- 역할: 이 파일은 해당 디렉터리나 모듈의 구조, 책임, 운영 규칙, 검증 기준을 설명한다.
- 기준: 파일 경로, 명령, API 이름, 세부 변경 이력은 위 영어 본문을 원문 기준으로 유지한다.
- 변경 시: 위 영어 본문을 수정하면 이 한국어 하단 섹션도 함께 최신 상태로 맞춘다.
