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
- Stay independent of note persistence, parser ownership, and editor surface objects. The document-derived inputs are
  measured visual-line count and measured row-width ratios produced by the geometry adapter after wrapping and
  rendered-block height have been applied.
- `ContentsEditorVisualLineMetrics` never inspects TextEdit geometry. It normalizes measured visual-line snapshots into
  minimap row count and width-ratio outputs.

## Verification

- `contentsMinimapLayoutMetrics_resolvesRuntimeVisibilityAndDesignRows`
- `contentsEditorVisualLineMetrics_expandsTallVisualBlocks`
- `qmlStructuredEditors_mountsEditorAndMinimapInDisplayLayout`
- `qmlContentsView_composesFigmaFrameFromLvrsParts`

## 한국어

이 섹션은 위 README 내용을 한국어로 확인하기 위한 하단 요약이다.

- 대상: `src/app/models/editor/minimap`
- 역할: 미니맵 폭, 표시 여부에 따른 폭, 런타임 행 수, Figma 기준 행 수 계산을 C++ 모델에 둔다.
- 기준: TextEdit/resource 표면 측정은 `geometry/ContentsEditorGeometryProvider`가 담당하고, `ContentsEditorVisualLineMetrics`는 측정된 visual line count와 row width ratio snapshot만 소비한다. `ContentsMinimapLayoutMetrics`는 그 결과를 미니맵 폭/행 수 정책으로 변환한다.
- 행별 폭: 실제 텍스트 줄 길이에 따른 width ratio는 geometry adapter가 측정하고, 미니맵 metrics는 이를 정규화해 `Minimap.qml`로 전달한다.
- 변경 시: 위 영어 본문을 수정하면 이 한국어 하단 섹션도 함께 최신 상태로 맞춘다.
