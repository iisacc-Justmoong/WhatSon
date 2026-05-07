# `src/app/models/editor/geometry/IContentsEditorGeometryProvider.hpp`

## Responsibility

Declares the interface boundary between editor chrome metrics and view-owned geometry objects.

## Public Surface

- `ContentsEditorGeometryMeasurement` carries a measured `QRectF` plus a `geometryAvailable` flag.
- `measureTextRange(...)` measures a visible text logical range.

## Contract

Consumers such as editor chrome metrics must use this interface instead of directly referencing TextEdit, cursor,
selection, resource overlay, or `QQuickItem` objects. The interface is read-only and cannot mutate editor source,
focus, cursor position, or selection state.

## 한국어

이 헤더는 거터/줄 번호 계산과 실제 에디터 표면 객체 사이의 인터페이스 경계를 선언한다. 계산 객체는 이
인터페이스만 보고, TextEdit/selection/cursor/resource 객체에 직접 결합하지 않는다.
