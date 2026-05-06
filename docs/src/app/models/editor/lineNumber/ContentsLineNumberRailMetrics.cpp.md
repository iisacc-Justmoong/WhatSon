# `src/app/models/editor/lineNumber/ContentsLineNumberRailMetrics.cpp`

## Responsibility

Implements logical line-number rail row construction outside QML.

## Behavior

- Normalizes renderer-owned block metadata only for de-duplication and resource-row detection.
- Splits row ranges from the full `logicalText` projection, so blank logical lines and tag-hidden source spans still
  receive the same line-number slots as the live editor text surface.
- Converts logical display offsets back to RAW source offsets through its internal `ContentsLogicalTextBridge`, using
  the supplied `sourceText` snapshot rather than a QML-provided offset table.
- Exposes `logicalLineRanges` for the geometry adapter, then consumes `geometryRows` value snapshots when building
  final gutter rows. This keeps actual view-object measurement outside the row-building policy.
- Validates measured row y positions before publishing them. If a measured snapshot is unavailable or collapses later
  rows back to the first row's y coordinate, the model places the row immediately below the previous resolved row
  instead of letting line numbers overlap.
- Resource blocks remain one logical row; their visible height is supplied as a measured row geometry snapshot.

## 한국어

이 구현은 QML에 있던 논리 줄 분할과 row y/height 생성을 C++로 이동한 것이다. 줄 번호 슬롯은 전체
`logicalText`에서 만들고, QML은 geometry row snapshot을 넘겨 측정값만 제공한다. 측정 snapshot이
일시적으로 같은 y를 반환해도 row를 이전 row 아래로 보정하여 2번 줄 번호가 1번 위치에 겹치지 않게 한다.
