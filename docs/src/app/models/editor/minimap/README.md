# `src/app/models/editor/minimap`

## Scope

Minimap layout calculation models for the editor domain.

## Files

- `ContentsMinimapLayoutMetrics.hpp`
- `ContentsMinimapLayoutMetrics.cpp`

## Contract

- Own minimap width, runtime row count, design row count, and visibility-to-width calculations.
- Accept LVRS token values from QML as primitive inputs, then resolve arithmetic and visibility behavior in C++.
- Keep `view/contents/Minimap.qml` presentation-only; it receives a row count and width from its host.
- Stay independent of note persistence and parser ownership. The only document-derived input is `logicalLineCount`.

## Verification

- `contentsMinimapLayoutMetrics_resolvesRuntimeVisibilityAndDesignRows`
- `qmlStructuredEditors_mountsGutterEditorAndMinimapInDisplayLayout`
- `qmlContentsView_composesFigmaFrameFromLvrsParts`

## 한국어

이 섹션은 위 README 내용을 한국어로 확인하기 위한 하단 요약이다.

- 대상: `src/app/models/editor/minimap`
- 역할: 미니맵 폭, 표시 여부에 따른 폭, 런타임 행 수, Figma 기준 행 수 계산을 C++ 모델에 둔다.
- 기준: QML은 LVRS 토큰과 `logicalLineCount`를 입력값으로 넘기고, 계산은 `ContentsMinimapLayoutMetrics`가 담당한다.
- 변경 시: 위 영어 본문을 수정하면 이 한국어 하단 섹션도 함께 최신 상태로 맞춘다.
