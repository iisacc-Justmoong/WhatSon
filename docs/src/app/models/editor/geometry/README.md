# `src/app/models/editor/geometry`

## Scope

C++ geometry adapter contracts for editor chrome models.

## Child Files

- `IContentsEditorGeometryProvider.hpp`
- `ContentsEditorGeometryProvider.hpp`
- `ContentsEditorGeometryProvider.cpp`

## Current Contract

- `IContentsEditorGeometryProvider` is the low-level measurement interface implemented by the adapter.
- `ContentsEditorGeometryProvider` is the QML-visible adapter that receives view-owned TextEdit/target items and
  translates `positionToRectangle(...)`, mapping, explicit structured resource visual block heights, and visual-line
  probes into measured value snapshots. Rendered HTML is not a geometry input.
- Line-number row placement measures ordinary rows from the plain logical display item. Atomic resource frame height
  comes from structured resource visual blocks computed by the inline resource presentation controller from resolved
  resource entries. The gutter metrics consumer uses that delta for placement only and keeps the resource allocation to
  one gutter-line height. Resource deltas compare the frame height to the next plain logical row's base-y advance, so
  the row after a frame lands on the frame bottom even when the hidden plain TextEdit line box has extra ascent/descent.
  If that next row base y is not yet measurable, the full frame height is used as the advance. The resolved frame bottom
  also clamps later measured row tops, so placeholder or probe rows reported inside a resource frame cannot become
  visible gutter anchors. Once a row is clamped out of the frame, the adapter advances the next minimum row top by the
  published row height so later text or blank logical rows cannot overlap the first post-resource row.
- Gutter and minimap metrics must depend on measured snapshot values only. They must not directly hold TextEdit,
  cursor, selection, resource overlay, or QQuickItem references.
- This directory owns measurement adaptation only. It does not mutate `.wsnbody`, drive cursor movement, own selection
  state, parse XML, or decide line-number row policy.

## Verification

- `contentsLineNumberRailMetrics_buildsRowsFromLogicalBlocks`
- `contentsEditorGeometryProvider_usesExplicitResourceVisualBlocks`
- `qmlInlineFormatEditor_projectsVisibleGeometryFromRenderedDisplay`
- `qmlInternalTypeRegistrar_usesLvrsManifestRegistration`

## 한국어

- 대상: `src/app/models/editor/geometry`
- 역할: 에디터 표면 지오메트리 측정을 C++ 인터페이스와 QML-visible adapter로 격리한다.
- 기준: 거터/줄 번호/미니맵 계산 객체는 측정 snapshot만 사용하고 TextEdit, cursor, selection, resource overlay
  객체를 직접 소유하거나 호출하지 않는다. 거터 일반 row는 plain logical geometry를 기준으로 삼고, atomic resource
  frame의 vertical delta는 rendered HTML 문자열 파싱이 아니라 structured resource visual block height를 사용한다.
  resource frame 내부로 측정된 probe row top은 frame bottom으로 clamp되어 내부 줄 번호 anchor로 노출되지 않는다.
  frame 밖으로 clamp된 row 이후 다음 row top은 방금 배치한 row height만큼 전진하므로 2번/3번 같은 연속 gutter
  번호가 같은 y에 겹치지 않는다.
- 변경 시: 이 경계가 깨지지 않도록 위 검증 항목과 관련 문서를 함께 갱신한다.
