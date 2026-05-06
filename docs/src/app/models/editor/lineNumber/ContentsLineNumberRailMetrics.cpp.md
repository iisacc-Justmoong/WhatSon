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
- Publishes each measured row independently. If a measured snapshot is unavailable, the model uses that row's line number
  as a simple line-height fallback; it does not use the previous row bottom to shift later gutter rows.
- Resource blocks remain one logical row and one gutter-line height. The measured frame height can place following rows
  through their own y snapshots, but it must not turn the resource block into a multi-line gutter allocation.

## 한국어

이 구현은 QML에 있던 논리 줄 분할과 row y/height 생성을 C++로 이동한 것이다. 줄 번호 슬롯은 전체
`logicalText`에서 만들고, QML은 geometry row snapshot을 넘겨 측정값만 제공한다. 각 row는 자기 y snapshot을
사용하며, 리소스 row가 크게 측정되더라도 그 높이를 거터 여러 줄로 환산하지 않고 한 줄 높이로 유지한다.
