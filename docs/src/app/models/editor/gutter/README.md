# `src/app/models/editor/gutter`

## Scope

Gutter layout calculation models for the editor domain.

## Files

- `ContentsGutterLayoutMetrics.hpp`
- `ContentsGutterLayoutMetrics.cpp`
- `ContentsGutterLineNumberGeometry.hpp`
- `ContentsGutterLineNumberGeometry.cpp`
- `ContentsGutterMarkerGeometry.hpp`
- `ContentsGutterMarkerGeometry.cpp`

## Contract

- Own gutter width, line-number count, inactive-line sentinel, line-number column, marker position, marker size, and
  icon-rail metric calculations.
- Own source-line y-position projection through `ContentsGutterLineNumberGeometry`.
- Map line numbers from RAW source physical line starts, the same way a text editor gutter numbers document lines.
  Parser/display block streams annotate those rows but do not create additional row count.
- Sample mounted editor geometry through `editorGeometryHost.lineStartRectangle(...)` whenever possible. Every line
  number row carries its RAW source span, HTML/logical line span, and resolved geometry row box together, so wrapped
  text, rendered resources, and live text layout decide each source line's y position and height.
- Convert sampled editor points through `mapTarget` when available, so the gutter receives viewport-relative y
  positions and scroll movement cannot leave line numbers fixed to the document origin.
- Never distribute remaining `editorContentHeight` into a resource row or terminal row. Resource rows become tall only
  when the mounted editor geometry places the next source line below the rendered resource frame.
- Treat gutter geometry as refreshable editor state. Note active-state changes, editor-session synchronization, RAW text
  edits, parser block refreshes, rendered resource updates, viewport movement, and rendered content-height changes may
  all request another projection pass.
- Own gutter marker projection through `ContentsGutterMarkerGeometry`: blue cursor marker from the current cursor line
  and yellow unsaved markers from current RAW text versus the last saved `.wsnbody` snapshot.
- Accept LVRS token values from QML as primitive inputs, then resolve arithmetic, fallback, and override behavior in C++.
- Keep `view/contents/Gutter.qml` presentation-only; it receives resolved metrics, line-number entries, and marker entries,
  then renders them without stacking or fallback layout logic.
- Stay independent of note persistence and parser ownership. The document-derived inputs are RAW source text, flattened
  display blocks, logical document blocks, logical line-start offsets, logical-to-source offsets, and parser/resource
  metadata used for editor-geometry refresh.

## Verification

- `contentsGutterLayoutMetrics_resolvesRuntimeAndDesignMetrics`
- `contentsGutterLineNumberGeometry_projectsFallbackLineYEntries`
- `contentsGutterLineNumberGeometry_samplesEditorGeometryForRawSourceLines`
- `contentsGutterLineNumberGeometry_keepsSingleRawPhysicalLineAsOneEditorLine`
- `contentsGutterLineNumberGeometry_mergesDisplayAndStructuredBlockStreams`
- `contentsGutterLineNumberGeometry_ignoresEditorContentHeightWithoutGeometry`
- `contentsGutterLineNumberGeometry_usesEditorGeometryDeltaForTallResourceRows`
- `contentsGutterLineNumberGeometry_ignoresRenderedResourceMetadataWithoutGeometry`
- `contentsGutterLineNumberGeometry_ignoresLogicalLineOffsetsForSourceLineRowCount`
- `contentsGutterLineNumberGeometry_prefersLogicalLineStartsOverSparseSourceMap`
- `contentsGutterLineNumberGeometry_publishesRawLogicalAndGeometryCoordinatesPerRow`
- `contentsGutterMarkerGeometry_marksCursorAndUnsavedLineSpans`
- `qmlStructuredEditors_mountsGutterEditorAndMinimapInDisplayLayout`
- `qmlInlineFormatEditor_projectsGutterGeometryFromVisibleDisplay`
- `qmlContentsView_composesFigmaFrameFromLvrsParts`

## 한국어

이 섹션은 위 README 내용을 한국어로 확인하기 위한 하단 요약이다.

- 대상: `src/app/models/editor/gutter`
- 역할: 거터 폭, 줄 번호 개수, 비활성 줄 번호, 줄 번호 칼럼 위치와 폭, source line 기반 라인 번호 y 좌표와 막대 좌표 계산을 C++ 모델에 둔다.
- 기준: QML은 LVRS 토큰, RAW source text, 표시용 평탄 블록, 논리 document block, logical line-start/source offset tables, parser/resource metadata, mounted editor geometry host와 gutter map target을 넘기고, 계산과 폴백은 `ContentsGutterLayoutMetrics`, `ContentsGutterLineNumberGeometry`, `ContentsGutterMarkerGeometry`가 담당한다. 라인 번호 개수는 RAW source의 물리 line start 기준이며, parser/display block은 해당 source line을 annotate할 뿐 row count를 늘리지 않는다. 각 라인 번호 entry는 RAW span, HTML/logical span, 실제 geometry box를 함께 가진다. y와 height는 code editor처럼 line index -> logical line start -> mounted editor geometry 순서로 구하고, 남는 content height를 특정 줄에 임의 배정하지 않는다.
- 변경 시: 위 영어 본문을 수정하면 이 한국어 하단 섹션도 함께 최신 상태로 맞춘다.
