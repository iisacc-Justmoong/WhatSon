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
- Resource blocks remain one logical row and one gutter-line height. The logical text bridge exposes each resource as
  one U+FFFC placeholder. The geometry adapter measures ordinary row y positions from the plain logical display item
  and applies only the rendered HTML resource image-height delta to following rows, so text after an image frame lands
  below the frame without asking the RichText overlay to map unrelated logical offsets. That delta is anchored to the
  next plain logical row's base y, so placeholder line-box height cannot pull the following gutter row upward. When the
  next row base y is not yet measurable, the geometry adapter uses the full frame height as the advance. If the probe
  later reports a row top still inside that frame, the adapter clamps it to the rendered frame bottom before the metrics
  object receives the snapshot.

## 한국어

이 구현은 QML에 있던 논리 줄 분할과 row y/height 생성을 C++로 이동한 것이다. 줄 번호 슬롯은 전체
`logicalText`에서 만들고, QML은 geometry row snapshot을 넘겨 측정값만 제공한다. 각 row는 plain logical geometry에서
얻은 자기 y snapshot을 사용한다. rendered mode에서는 rendered HTML의 resource image-height delta만 이후 row에 더하고,
그 delta는 다음 plain logical row의 base y를 기준으로 계산한다. 다음 row base y가 아직 측정되지 않으면 frame
height 전체를 advance로 사용한다. probe가 resource frame 내부 y를 반환한 row는 geometry adapter에서 frame bottom으로
고정된 뒤 전달된다. 리소스 row가 크게 측정되더라도 그 높이를 거터
여러 줄로 환산하지 않고 한 줄 높이로 유지한다.
