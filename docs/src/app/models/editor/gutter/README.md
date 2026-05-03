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
- Own document-block y-position projection through `ContentsGutterLineNumberGeometry`.
- Map line numbers from the flattened visible block stream and parser-owned logical block stream rather than
  RichText/display geometry sampling. RAW newline order is only the fallback when block metadata is unavailable.
- Merge complementary block ranges from both streams so a visible resource projection and structured text projection can
  both contribute rows during note-state refresh.
- Treat `resource` entries as one atomic gutter row. The row can become taller to match the rendered image frame, but it
  must not expand into synthetic RAW tag text rows.
- Treat gutter geometry as refreshable editor state. Note active-state changes, editor-session synchronization, RAW text
  edits, parser block refreshes, rendered resource updates, viewport movement, and rendered content-height changes may
  all request another projection pass.
- Own gutter marker projection through `ContentsGutterMarkerGeometry`: blue cursor marker from the current cursor line
  and yellow unsaved markers from current RAW text versus the last saved `.wsnbody` snapshot.
- Accept LVRS token values from QML as primitive inputs, then resolve arithmetic, fallback, and override behavior in C++.
- Keep `view/contents/Gutter.qml` presentation-only; it receives resolved metrics, line-number entries, and marker entries,
  then renders them without stacking or fallback layout logic.
- Stay independent of note persistence and parser ownership. The document-derived inputs are RAW source text, flattened
  display blocks, logical document blocks, and parser/resource metadata used for resource-row height reconciliation.

## Verification

- `contentsGutterLayoutMetrics_resolvesRuntimeAndDesignMetrics`
- `contentsGutterLineNumberGeometry_projectsFallbackLineYEntries`
- `contentsGutterLineNumberGeometry_usesRawLineOffsetsWithoutGeometrySampling`
- `contentsGutterLineNumberGeometry_readsStructuredBlocksWhenRawHasSinglePhysicalLine`
- `contentsGutterLineNumberGeometry_mergesDisplayAndStructuredBlockStreams`
- `contentsGutterLineNumberGeometry_mapsSecondRawLineAfterTallFirstResource`
- `contentsGutterLineNumberGeometry_expandsResourceRowsToRenderedContentHeight`
- `contentsGutterLineNumberGeometry_assignsRenderedResourceHeightToVisibleImageRows`
- `contentsGutterLineNumberGeometry_ignoresDuplicateDisplaySamples`
- `contentsGutterMarkerGeometry_marksCursorAndUnsavedLineSpans`
- `qmlStructuredEditors_mountsGutterEditorAndMinimapInDisplayLayout`
- `qmlInlineFormatEditor_projectsGutterGeometryFromVisibleDisplay`
- `qmlContentsView_composesFigmaFrameFromLvrsParts`

## 한국어

이 섹션은 위 README 내용을 한국어로 확인하기 위한 하단 요약이다.

- 대상: `src/app/models/editor/gutter`
- 역할: 거터 폭, 줄 번호 개수, 비활성 줄 번호, 줄 번호 칼럼 위치와 폭, 실제 라인 번호 y 좌표와 막대 좌표 계산을 C++ 모델에 둔다.
- 기준: QML은 LVRS 토큰, RAW source text, 표시용 평탄 블록, 논리 document block, parser/resource metadata를 넘기고, 계산과 폴백은 `ContentsGutterLayoutMetrics`, `ContentsGutterLineNumberGeometry`, `ContentsGutterMarkerGeometry`가 담당한다. 라인 번호 y는 표시 geometry가 아니라 실제 표시될 블록 순서를 기준으로 누적 계산하며, 두 블록 스트림의 누락 구간은 병합하고 RAW newline은 블록 metadata가 없을 때의 폴백이다. 노트 active 상태, 세션 동기화, RAW 편집, parser/resource refresh, viewport 변화에 따라 거터 계산은 반복 실행될 수 있다.
- 변경 시: 위 영어 본문을 수정하면 이 한국어 하단 섹션도 함께 최신 상태로 맞춘다.
