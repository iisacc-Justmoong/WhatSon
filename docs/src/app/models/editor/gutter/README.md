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
- Own line-number y-position projection from the currently visible editor display geometry through
  `ContentsGutterLineNumberGeometry`.
- Own gutter marker projection through `ContentsGutterMarkerGeometry`: blue cursor marker from the current cursor line
  and yellow unsaved markers from current RAW text versus the last saved `.wsnbody` snapshot.
- Accept LVRS token values from QML as primitive inputs, then resolve arithmetic, fallback, and override behavior in C++.
- Keep `contents/Gutter.qml` presentation-only; it receives resolved metrics, line-number entries, and marker entries,
  then renders them without stacking or fallback layout logic.
- Stay independent of note persistence and parser ownership. The only document-derived input is `logicalLineCount`.

## Verification

- `contentsGutterLayoutMetrics_resolvesRuntimeAndDesignMetrics`
- `contentsGutterLineNumberGeometry_projectsFallbackLineYEntries`
- `contentsGutterLineNumberGeometry_samplesVisibleDisplayOffsets`
- `contentsGutterMarkerGeometry_marksCursorAndUnsavedLineSpans`
- `qmlStructuredEditors_mountsGutterEditorAndMinimapInDisplayLayout`
- `qmlInlineFormatEditor_projectsGutterGeometryFromVisibleDisplay`
- `qmlContentsView_composesFigmaFrameFromLvrsParts`

## 한국어

이 섹션은 위 README 내용을 한국어로 확인하기 위한 하단 요약이다.

- 대상: `src/app/models/editor/gutter`
- 역할: 거터 폭, 줄 번호 개수, 비활성 줄 번호, 줄 번호 칼럼 위치와 폭, 실제 라인 번호 y 좌표와 막대 좌표 계산을 C++ 모델에 둔다.
- 기준: QML은 LVRS 토큰과 현재 보이는 에디터 표시 geometry 훅을 입력값으로 넘기고, 계산과 폴백은 `ContentsGutterLayoutMetrics`, `ContentsGutterLineNumberGeometry`, `ContentsGutterMarkerGeometry`가 담당한다.
- 변경 시: 위 영어 본문을 수정하면 이 한국어 하단 섹션도 함께 최신 상태로 맞춘다.
