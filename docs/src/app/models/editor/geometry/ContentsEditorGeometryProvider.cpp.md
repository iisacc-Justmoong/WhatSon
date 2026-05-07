# `src/app/models/editor/geometry/ContentsEditorGeometryProvider.cpp`

## Responsibility

Implements the editor-surface geometry adapter behind `IContentsEditorGeometryProvider`.

## Behavior

- Reads `positionToRectangle(...)` from the supplied TextEdit-like geometry item.
- Maps measured points into the supplied target item coordinate system when both sides are `QQuickItem` instances.
- Measures line-number text rows from the plain logical display item, including U+FFFC resource placeholders, so ordinary
  rows keep independent y snapshots even when rendered RichText geometry is unstable.
- Reads rendered resource-frame geometry from the separate resource item. A resource row is bounded by the next rendered
  row top when available, otherwise by resource `contentHeight` capped by the next plain logical row top. The resulting
  resource visual-height delta is applied to later rows, while the line-number metrics layer still clamps the resource's
  visible gutter allocation to one line.
- Probes the visible item with `positionAt(...)` and `positionToRectangle(...)` to produce minimap row-width ratios.
- Converts logical line ranges into `lineNumberGeometryRows` value snapshots.
- Emits `geometryChanged()` whenever a bound view item, logical range list, or primitive measurement input changes.

## Non-Goals

The implementation does not decide gutter numbering, parse RAW XML, mutate `.wsnbody`, move the cursor, or control text
selection. Those responsibilities remain outside this adapter.

## 한국어

이 구현은 TextEdit/resource 표면의 측정 호출을 인터페이스 결과로 바꾸는 adapter다. 거터 일반 row는 plain
logical display geometry로 측정하고, rendered resource frame은 이후 row의 vertical delta 계산에만 사용한다.
커서와 selection에는 개입하지 않고, 거터 row 정책과 미니맵 row 정책도 각각의 metrics 객체에 남긴다.
