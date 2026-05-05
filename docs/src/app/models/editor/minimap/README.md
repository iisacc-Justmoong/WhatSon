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
- Stay independent of note persistence and parser ownership. The only document-derived input is `visualLineCount`,
  measured from the live editor surface after wrapping and rendered-block height have been applied.
- Per-row minimap width ratios are measured by the QML editor host from visible text geometry and passed directly to
  `Minimap.qml`; this C++ model owns rail and row-count arithmetic, not per-row text measurement.

## Verification

- `contentsMinimapLayoutMetrics_resolvesRuntimeVisibilityAndDesignRows`
- `qmlStructuredEditors_mountsEditorAndMinimapInDisplayLayout`
- `qmlContentsView_composesFigmaFrameFromLvrsParts`

## 한국어

이 섹션은 위 README 내용을 한국어로 확인하기 위한 하단 요약이다.

- 대상: `src/app/models/editor/minimap`
- 역할: 미니맵 폭, 표시 여부에 따른 폭, 런타임 행 수, Figma 기준 행 수 계산을 C++ 모델에 둔다.
- 기준: QML은 LVRS 토큰과 wrap 이후 실제 에디터 표시 줄 수인 `visualLineCount`를 입력값으로 넘기고, 계산은 `ContentsMinimapLayoutMetrics`가 담당한다. 리소스 프레임 등 큰 표시 블록은 세로 표시 높이를 줄 높이로 환산한 값까지 포함한다.
- 행별 폭: 실제 텍스트 줄 길이에 따른 width ratio는 QML 에디터 host가 표시 지오메트리에서 측정해 `Minimap.qml`로 직접 전달한다.
- 변경 시: 위 영어 본문을 수정하면 이 한국어 하단 섹션도 함께 최신 상태로 맞춘다.
