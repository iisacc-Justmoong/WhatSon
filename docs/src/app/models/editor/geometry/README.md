# `src/app/models/editor/geometry`

## Scope

C++ geometry adapter contracts for editor chrome models.

## Child Files

- `IContentsEditorGeometryProvider.hpp`
- `ContentsEditorGeometryProvider.hpp`
- `ContentsEditorGeometryProvider.cpp`

## Current Contract

- `IContentsEditorGeometryProvider` is the low-level measurement interface implemented by the adapter.
- `ContentsEditorGeometryProvider` is the QML-visible adapter that receives view-owned TextEdit/resource/target items
  and translates their `positionToRectangle(...)`, mapping, rendered HTML resource image heights, and visual-line probes
  into measured value snapshots.
- Line-number row placement measures ordinary rows from the plain logical display item. The rendered resource item is
  not queried for following text-row offsets. Atomic resource frame height comes from the rendered HTML image height
  and falls back to one line instead of whole-document rendered `contentHeight`. The gutter metrics consumer uses that
  delta for placement only and keeps the resource allocation to one gutter-line height. Resource deltas compare the
  frame height to the next plain logical row's base-y advance, so the row after a frame lands on the frame bottom even
  when the hidden plain TextEdit line box has extra ascent/descent. If that next row base y is not yet measurable, the
  full frame height is used as the advance. The resolved frame bottom also clamps later measured row tops, so placeholder
  or probe rows reported inside a resource frame cannot become visible gutter anchors.
- Gutter and minimap metrics must depend on measured snapshot values only. They must not directly hold TextEdit,
  cursor, selection, resource overlay, or QQuickItem references.
- This directory owns measurement adaptation only. It does not mutate `.wsnbody`, drive cursor movement, own selection
  state, parse XML, or decide line-number row policy.

## Verification

- `contentsLineNumberRailMetrics_buildsRowsFromLogicalBlocks`
- `qmlInlineFormatEditor_projectsVisibleGeometryFromRenderedDisplay`
- `qmlInternalTypeRegistrar_usesLvrsManifestRegistration`

## 한국어

- 대상: `src/app/models/editor/geometry`
- 역할: 에디터 표면 지오메트리 측정을 C++ 인터페이스와 QML-visible adapter로 격리한다.
- 기준: 거터/줄 번호/미니맵 계산 객체는 측정 snapshot만 사용하고 TextEdit, cursor, selection, resource overlay
  객체를 직접 소유하거나 호출하지 않는다. 거터 일반 row는 plain logical geometry를 기준으로 삼고 rendered HTML의
  resource image height만 atomic resource frame의 vertical delta 계산에 사용한다. resource frame 내부로 측정된
  probe row top은 frame bottom으로 clamp되어 내부 줄 번호 anchor로 노출되지 않는다.
- 변경 시: 이 경계가 깨지지 않도록 위 검증 항목과 관련 문서를 함께 갱신한다.
