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
  and translates their `positionToRectangle(...)`, mapping, resource `contentHeight`, and visual-line probes into
  measured value snapshots.
- Line-number row placement measures ordinary rows from the plain logical display item. The rendered resource item is
  used only to resolve the visual-height delta of atomic resource frames, bounded by the next rendered row top when
  available or by resource `contentHeight` capped by the next plain logical row top. The gutter metrics consumer uses
  that delta for placement only and keeps the resource allocation to one gutter-line height.
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
  객체를 직접 소유하거나 호출하지 않는다. 거터 일반 row는 plain logical geometry를 기준으로 삼고 resource overlay는
  atomic resource frame의 vertical delta 계산에만 사용한다.
- 변경 시: 이 경계가 깨지지 않도록 위 검증 항목과 관련 문서를 함께 갱신한다.
