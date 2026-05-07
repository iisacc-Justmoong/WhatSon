# `src/app/models/editor/lineNumber/ContentsLineNumberRailMetrics.hpp`

## Responsibility

Declares the QML-visible C++ metrics object for the editor line-number rail.

## Public Surface

- Inputs: `sourceText`, compatibility `logicalText`, `normalizedHtmlBlocks`, `geometryRows`, `textLineHeight`,
  `geometryWidth`, and `displayContentHeight`.
- Output: `rows`, a `QVariantList` of row maps with `number`, `sourceStart`, `sourceEnd`, `y`, and `height`.
- Output: `logicalLineRanges`, source-derived source/logical ranges that an external geometry adapter can measure.
- Slot: `requestRowsRefresh()` emits `rowsChanged()` when the view geometry has changed without a source snapshot
  change.
- Test helper: `logicalLineRanges()` exposes the source-derived logical ranges before view geometry is applied.

## 한국어

이 헤더는 QML에서 생성할 수 있는 줄 번호 rail C++ 객체의 계약을 선언한다. 정상 QML 경로는 `sourceText`,
renderer metadata, geometry row snapshot을 바인딩하고, source offset 변환은 이 C++ 객체가 내부 logical text
bridge로 계산한다. `logicalText` property는 이전 호출자 호환을 위해 남아 있지만 정상 row 계산 입력이 아니다.
TextEdit/cursor/selection/resource 객체는 이 헤더의 public surface에 노출되지 않는다.
