# `src/app/qml`

## Role
This directory contains the root LVRS application shell and QML-side interaction surfaces.

The core rule in this directory is that visual composition belongs here, while persistence and mutation logic stay in C++ model-domain controllers or narrow bridge/helper objects under `src/app/models`.

## Important Files
- `Main.qml`: the root `LV.ApplicationWindow`, route shell, and root context-object consumer.
- `DesignTokens.qml`: QML-side design token aggregation.
- `contents/ContentsView.qml`: standalone Figma `ContentsView` frame that embeds the sibling gutter, editor, and
  minimap QML parts using `LV.Theme` tokens for colors, spacing, typography, and fixed rails.

## LVRS Token Rule
- QML view files must express reusable colors, transparency, spacing, typography, and fixed UI extents through
  `LV.Theme` tokens or token compositions.
- Direct visual literals such as `#...`, `Qt.rgba(...)`, `color: "transparent"`, `spacing: 0`, and raw
  `font.pixelSize` values are reserved for non-visual data/algorithmic cases only.

## Ownership Model
The C++ composition root applies `WhatSonQmlContextBinder` before root QML load, exposing required runtime objects as
LVRS context-object bindings. `Main.qml` consumes those objects directly and forwards explicit dependencies to child
views.

## Why This Directory Is Important
If a runtime object exists in C++ but behaves incorrectly in the UI, this directory is usually where the mismatch becomes visible first.

## 한국어

이 섹션은 위 README 내용을 한국어로 확인하기 위한 하단 요약이다.

- 대상: ``src/app/qml`` (`docs/src/app/qml/README.md`)
- 위치: `docs/src/app/qml`
- 역할: 이 파일은 해당 디렉터리나 모듈의 구조, 책임, 운영 규칙, 검증 기준을 설명한다.
- 기준: 파일 경로, 명령, API 이름, 세부 변경 이력은 위 영어 본문을 원문 기준으로 유지한다.
- 변경 시: 위 영어 본문을 수정하면 이 한국어 하단 섹션도 함께 최신 상태로 맞춘다.
