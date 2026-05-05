# `src/app/models/editor/geometry/ContentsEditorGeometryProvider.hpp`

## Responsibility

Declares the QML-visible implementation of `IContentsEditorGeometryProvider`.

## Public Surface

- Inputs: `textItem`, `resourceItem`, `targetItem`, `visualItem`, logical line ranges, and primitive visual geometry
  values supplied by the QML view layer.
- Signals: per-input change signals plus `geometryChanged()` so consumers can refresh cached rows when the measured
  surface changes.
- Outputs: `lineNumberGeometryRows`, `visualLineCount`, and `visualLineWidthRatios` snapshots for independent chrome
  metrics.
- Interface methods: low-level text range measurement, resource range measurement, and resource content-height lookup.

## Boundary

This object is an adapter, not a line-number policy object. It may inspect view-owned geometry primitives, but it does
not know about logical row numbering, source mutation, cursor movement, or selection ownership.

## 한국어

이 헤더는 QML에서 생성할 수 있는 지오메트리 adapter를 선언한다. 뷰 객체는 여기까지만 들어오며,
거터/미니맵 계산 객체에는 측정 snapshot 값만 전달된다.
