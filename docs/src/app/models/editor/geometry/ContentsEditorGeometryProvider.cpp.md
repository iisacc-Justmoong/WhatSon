# `src/app/models/editor/geometry/ContentsEditorGeometryProvider.cpp`

## Responsibility

Implements the editor-surface geometry adapter behind `IContentsEditorGeometryProvider`.

## Behavior

- Reads `positionToRectangle(...)` from the supplied TextEdit-like geometry item.
- Maps measured points into the supplied target item coordinate system when both sides are `QQuickItem` instances.
- Measures line-number text rows from the plain logical display item, including U+FFFC resource placeholders, so ordinary
  rows keep independent y snapshots even when rendered RichText geometry is unstable.
- Reads rendered resource-frame image heights from the supplied rendered HTML. The resulting resource visual-height
  delta is applied to later rows, while the line-number metrics layer still clamps the resource's visible gutter
  allocation to one line. If no rendered resource height is available, the adapter falls back to one logical line rather
  than using whole-document rendered `contentHeight`.
- The resource delta is calculated against the next plain logical row's measured base y, not against the resource
  placeholder rectangle height. This keeps the row after an image frame anchored to `resourceRow.y + frameHeight` even
  when the hidden plain TextEdit line box height differs from its line-to-line advance. If the next plain row base y is
  not yet measurable, the full resource frame height is used as the advance instead of subtracting placeholder line
  height.
- Once a resource row is resolved, its rendered frame bottom becomes an exclusion boundary for later gutter geometry.
  Any probe row whose measured top still falls inside that frame is clamped to the frame bottom instead of becoming an
  internal line-number anchor.
- Inline resource HTML must keep the frame paragraph at zero line height and top-align the image, so that encoded image
  height is the same bottom anchor the RichText surface paints.
- Probes the visible item with `positionAt(...)` and `positionToRectangle(...)` to produce minimap row-width ratios.
- Converts logical line ranges into `lineNumberGeometryRows` value snapshots.
- Emits `geometryChanged()` whenever a bound view item, logical range list, or primitive measurement input changes.

## Non-Goals

The implementation does not decide gutter numbering, parse RAW XML, mutate `.wsnbody`, move the cursor, or control text
selection. Those responsibilities remain outside this adapter.

## 한국어

이 구현은 TextEdit/resource 표면의 측정 호출을 인터페이스 결과로 바꾸는 adapter다. 거터 일반 row는 plain
logical display geometry로 측정하고, rendered HTML의 resource image height는 이후 row의 vertical delta 계산에만
사용한다. inline resource HTML은 paragraph line-height를 0으로 고정해 image height와 실제 frame bottom을
일치시킨다. resource delta는 placeholder rectangle height가 아니라 다음 plain logical row의 base y와 비교해
계산하므로 resource 뒤 첫 row가 frame bottom에 붙는다. 다음 row base y가 아직 0으로만 측정되면 placeholder
line-height를 빼지 않고 frame height 전체를 advance로 쓴다. resource frame bottom은 이후 row의 최소 y로도
고정되므로 probe가 프레임 내부 placeholder row를 반환해도 그 row는 내부 line-number anchor가 되지 않는다.
resource height를 얻지 못해도 전체 rendered `contentHeight`를 fallback으로 쓰지 않는다.
커서와 selection에는 개입하지 않고, 거터 row 정책과 미니맵 row 정책도 각각의 metrics 객체에 남긴다.
