# `src/app/models/editor/minimap`

## Scope

Minimap layout calculation models for the editor domain.

## Files

- `ContentsMinimapLayoutMetrics.hpp`
- `ContentsMinimapLayoutMetrics.cpp`
- `ContentsEditorVisualLineMetrics.hpp`
- `ContentsEditorVisualLineMetrics.cpp`

## Contract

- Own minimap width, runtime row count, design row count, and visibility-to-width calculations.
- Accept LVRS token values from QML as primitive inputs, then resolve arithmetic and visibility behavior in C++.
- Keep `view/contents/Minimap.qml` presentation-only; it receives a row count and width from its host.
- Stay independent of note persistence and parser ownership. The only document-derived input is `visualLineCount`,
  measured from the live editor surface after wrapping and rendered-block height have been applied.
- Per-row minimap width ratios are measured by `ContentsEditorVisualLineMetrics` from visible TextEdit geometry.
  QML only passes the visible TextEdit object and primitive geometry inputs.

## Verification

- `contentsMinimapLayoutMetrics_resolvesRuntimeVisibilityAndDesignRows`
- `contentsEditorVisualLineMetrics_expandsTallVisualBlocks`
- `qmlStructuredEditors_mountsEditorAndMinimapInDisplayLayout`
- `qmlContentsView_composesFigmaFrameFromLvrsParts`

## 한국어

이 섹션은 위 README 내용을 한국어로 확인하기 위한 하단 요약이다.

- 대상: `src/app/models/editor/minimap`
- 역할: 미니맵 폭, 표시 여부에 따른 폭, 런타임 행 수, Figma 기준 행 수 계산을 C++ 모델에 둔다.
- 기준: QML은 LVRS 토큰, 표시 중인 TextEdit 객체, primitive geometry 값만 넘기고, wrap 이후 실제 표시 줄 수와 행별 width ratio는 `ContentsEditorVisualLineMetrics`가 계산한다. `ContentsMinimapLayoutMetrics`는 그 결과를 미니맵 폭/행 수 정책으로 변환한다.
- 행별 폭: 실제 텍스트 줄 길이에 따른 width ratio는 C++ `ContentsEditorVisualLineMetrics`가 표시 지오메트리에서 측정해 `Minimap.qml`로 전달된다.
- 변경 시: 위 영어 본문을 수정하면 이 한국어 하단 섹션도 함께 최신 상태로 맞춘다.
