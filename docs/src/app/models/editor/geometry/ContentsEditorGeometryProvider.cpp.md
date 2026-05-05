# `src/app/models/editor/geometry/ContentsEditorGeometryProvider.cpp`

## Responsibility

Implements the editor-surface geometry adapter behind `IContentsEditorGeometryProvider`.

## Behavior

- Reads `positionToRectangle(...)` from the supplied TextEdit-like geometry item.
- Maps measured points into the supplied target item coordinate system when both sides are `QQuickItem` instances.
- Reads resource `contentHeight` for atomic resource frame rows.
- Probes the visible item with `positionAt(...)` and `positionToRectangle(...)` to produce minimap row-width ratios.
- Converts logical line ranges into `lineNumberGeometryRows` value snapshots.
- Emits `geometryChanged()` whenever a bound view item, logical range list, or primitive measurement input changes.

## Non-Goals

The implementation does not decide gutter numbering, parse RAW XML, mutate `.wsnbody`, move the cursor, or control text
selection. Those responsibilities remain outside this adapter.

## 한국어

이 구현은 TextEdit/resource 표면의 측정 호출을 인터페이스 결과로 바꾸는 adapter다. 커서와 selection에는
개입하지 않고, 거터 row 정책과 미니맵 row 정책도 각각의 metrics 객체에 남긴다.
