# `src/app/qml/contents`

## Role
This directory hosts standalone contents-surface view artifacts that mirror Figma content frames before they are wired
into the runtime editor stack.

## Current Files
- `ContentsView.qml`: Figma node `155:4561` root frame and LVRS layout host.
- `Gutter.qml`: transparent line-number rail plus cursor/unsaved markers for Figma node `155:5345`.
- `EditorView.qml`: read-only text projection for Figma node `155:5352`.
- `Minimap.qml`: minimap row rail for Figma node `352:8626`.

## Boundary
- The view is presentation-only.
- It does not own note persistence, parser state, or editor mutation authority.
- Every QML file imports LVRS and expresses colors, spacing, typography, and fixed rails through `LV.Theme` tokens.
- Figma dimensions are preserved through token compositions instead of literal pixel values.
- Gutter and minimap arithmetic is delegated to `ContentsGutterLayoutMetrics` and
  `ContentsMinimapLayoutMetrics`; the QML files render resolved values.
- Gutter line-number y projection is delegated to `ContentsGutterLineNumberGeometry`, so the standalone frame and the
  runtime editor share the same line-entry contract.
- Gutter marker projection is delegated to `ContentsGutterMarkerGeometry`, so cursor and unsaved-line markers share the
  same coordinate system as line labels.
- `ContentsView.qml` owns the Figma `155:5344` `LV.HStack` contract and orders children left-to-right as
  `Gutter.qml`, `EditorView.qml`, and `Minimap.qml`.

## Tests
- `test/cpp/suites/qml_contents_view_tests.cpp` locks the four-file composition contract and the read-only editor
  projection boundary.

## 한국어

이 섹션은 위 README 내용을 한국어로 확인하기 위한 하단 요약이다.

- 대상: ``src/app/qml/contents`` (`docs/src/app/qml/contents/README.md`)
- 위치: `docs/src/app/qml/contents`
- 역할: 이 파일은 해당 디렉터리나 모듈의 구조, 책임, 운영 규칙, 검증 기준을 설명한다.
- 배치: `ContentsView.qml`은 Figma `155:5344` `LV.HStack` 안에서 `Gutter.qml`, `EditorView.qml`, `Minimap.qml` 순서를 유지한다.
- 막대: 커서 위치는 파란 막대, 저장되지 않은 현재 RAW 줄은 노란 막대로 표시하며, 좌표 계산은 `ContentsGutterMarkerGeometry`가 담당한다.
- 배경: 거터는 별도 패널 배경을 칠하지 않고 에디터 표면과 같은 배경을 공유한다.
- 기준: 파일 경로, 명령, API 이름, 세부 변경 이력은 위 영어 본문을 원문 기준으로 유지한다. 거터와 미니맵 계산 및 거터 줄 번호 y 좌표는 C++ 모델 계층에서 담당한다.
- 변경 시: 위 영어 본문을 수정하면 이 한국어 하단 섹션도 함께 최신 상태로 맞춘다.
