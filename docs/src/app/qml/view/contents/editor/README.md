# `src/app/qml/view/contents/editor`

## Scope

Editor-facing QML view components for the center content surface.

## Child Files

- `ContentsDisplaySurfaceHost.qml`
- `ContentsDisplayView.qml`
- `ContentsInlineFormatEditor.qml`
- `ContentsLineNumberRail.qml`
- `ContentsResourceEditorView.qml`
- `ContentsResourceViewer.qml`
- `ContentsStructuredDocumentFlow.qml`

## Current Pipeline

- `ContentsDisplayView.qml` mounts the selected note RAW body into `ContentsEditorSessionController`.
- `ContentsEditorPresentationProjection` converts that RAW body into editor HTML plus renderer-owned block metadata.
- `ContentsEditorVisualLineMetrics` measures visible editor lines for the minimap, and
  `ContentsMinimapLayoutMetrics` resolves minimap chrome metrics under `src/app/models/editor/minimap`.
- `ContentsDisplayView.qml` owns the visible editor chrome layout and composes the parent `view/contents` minimap with
  `ContentsStructuredDocumentFlow.qml` in one `LV.HStack`.
- `ContentsDisplayView.qml` also mounts `ContentsLineNumberRail.qml` inside the same scrollable document content item as
  `ContentsStructuredDocumentFlow.qml`, so gutter rows and editor body rows share one y-coordinate system.
- `ContentsStructuredDocumentFlow.qml` consumes `editorSurfaceHtml`, `logicalText`, `htmlTokens`, and
  `normalizedHtmlBlocks`, and passes `logicalText` plus the logical cursor position to the inline editor's geometry
  probe so the overlay cursor measures visible text instead of RichText HTML tag positions. The same renderer-owned
  `normalizedHtmlBlocks` stream is forwarded as selection metadata so resource selection uses the iiHtmlBlock display
  block span as one atomic block. It also binds explicit formatting and body-tag shortcuts to the C++ tag insertion
  controller so `Cmd/Ctrl+B/I/U`, `Cmd/Ctrl+Shift+E` for highlight, and body commands such as callout/agenda insert or
  wrap proprietary RAW tags before the normal session persistence path runs. These commands read the live inline editor
  buffer when constructing payloads so consecutive tag commands do not combine fresh selection offsets with stale parent
  `sourceText` bindings.
- The center document slot owns a vertical `Flickable` viewport around `ContentsStructuredDocumentFlow.qml`; long note
  bodies scroll inside that viewport.
- The gutter row list is driven by `ContentsStructuredDocumentFlow.editorLogicalGutterRows`, which is resolved by the
  C++ `ContentsLineNumberRailMetrics` object mounted inside `ContentsInlineFormatEditor.qml`. Each gutter row
  represents one logical source/display block line. Wrapped text does not increment the number count, but the row
  height is measured from the live editor geometry so a long paragraph that wraps onto three visible lines gives one
  numbered gutter row with three visible lines of height. Atomic resource frames count as one logical row while using
  the rendered frame height.
- The minimap row count is driven by `ContentsStructuredDocumentFlow.editorVisualLineCount`, which comes from the C++
  `ContentsEditorVisualLineMetrics` object measuring the live wrapped editor surface. A single source tag or paragraph
  that wraps onto two visible editor lines therefore produces two minimap rows. Tall rendered blocks such as resource
  frames use their visible content height divided by the editor line height, so their minimap footprint follows the
  amount of vertical space they occupy.
- The minimap row widths are driven by `ContentsStructuredDocumentFlow.editorVisualLineWidthRatios`. Text rows use the
  visible line length measured from editor text geometry by `ContentsEditorVisualLineMetrics`; height-derived rows that
  cannot be probed from text geometry remain full width.
- The minimap can be dragged vertically as a compact scrollbar. `Minimap.qml` emits normalized scroll ratios and
  `ContentsDisplayView.qml` maps those ratios onto the center editor `Flickable.contentY`.
- `ContentsInlineFormatEditor.qml` keeps editing on an `LV.TextEditor` plain-text buffer while displaying the read-side
  RichText overlay. When native selection is active, that overlay stays visible so WYSIWYG text and resource frames do
  not collapse back to RAW tags; the underlying editor still owns the source range and keeps native selection highlight
  visible for ranges that contain visible logical content while RAW selected glyphs stay transparent. Tag-only source
  ranges, such as empty formatting wrappers, do not paint native or rendered selection because those tags produce no
  visible editor content. The rendered overlay mirrors text selection in logical coordinates, and resource spans from
  iiHtmlBlock metadata paint as one atomic block-level selection rectangle. Resource-backed overlays also stay pinned
  during ordinary native typing/composition turns, so a refresh gap cannot reveal the RAW `<resource ... />` tag in
  place of the frame. The editor paints an explicit blinking cursor above the overlay while the underlying RAW text and
  native RAW cursor delegate are hidden. The rendered surface is stacked below the transparent native editor. While that
  overlay is visible, a thin pointer bridge maps mouse-drag hit testing through `displayGeometryText` and
  `logicalToSourceOffsets`, then restores the matching RAW source selection on `LV.TextEditor`; this keeps sub-word
  selection precise even when hidden source tags would otherwise distort the transparent RAW text geometry. The bridge
  is inactive during IME composition. Plain-source and keyboard/modifier selection remain on the native text-edit path.
  If native cursor movement lands inside a hidden inline formatting tag, the inline editor normalizes that RAW cursor to
  the adjacent safe source boundary so arrow traversal skips zero-width tag bytes. Programmatic text replacement is
  routed through the inline editor controller so focused native selection can defer host-side surface refresh. Inline
  resource frames are rendered at 100% of the editor text-column width. The editor body has no top inset or overlay
  padding, so the first text line starts at the top of the document slot.
- Resource-backed center-surface browsing is handled by `ContentsResourceEditorView.qml` and `ContentsResourceViewer.qml`.

QML in this directory must stay presentation-only. XML parsing, HTML tokenization, block object construction, and RAW
source mutation policy remain in C++ model/renderer objects.

## 한국어

이 섹션은 위 README 내용을 한국어로 확인하기 위한 하단 요약이다.

- 대상: ``src/app/qml/view/contents/editor`` (`docs/src/app/qml/view/contents/editor/README.md`)
- 위치: `docs/src/app/qml/view/contents/editor`
- 역할: 이 파일은 해당 디렉터리나 모듈의 구조, 책임, 운영 규칙, 검증 기준을 설명하며 본문 편집기는 `LV.TextEditor` 기준이다.
- 배치: 실제 노트 편집 화면은 `ContentsDisplayView.qml`의 `LV.HStack`에서 본문 편집기와 미니맵 순서로 표시한다. 미니맵 계산은 C++ 모델 계층에서 수행한다.
- 기준: 파일 경로, 명령, API 이름, 세부 변경 이력은 위 영어 본문을 원문 기준으로 유지한다.
- selection: 본문 텍스트 선택 중에도 RichText overlay를 유지한다. 선택 source range는 `LV.TextEditor`가
  소유하고 visible logical content가 있는 범위에 대해서만 native selection highlight를 표시한다. 단 RAW
  selected glyph는 투명하게 유지한다. 빈 formatting wrapper처럼 태그만 포함된 source range는 표시할 본문이
  없으므로 native/rendered selection을 칠하지 않는다. 렌더된 텍스트 선택은 overlay의 logical selection으로
  동기화하고, 리소스는 iiHtmlBlock display block 하나로 판정해 atomic block 선택 rect 하나만 표시한다.
- cursor: RichText overlay가 보이는 동안에는 논리 텍스트 좌표 기반의 표면 커서만 overlay 위에 그리고, 아래의
  네이티브 RAW 커서 delegate는 숨긴다.
- top inset: 본문 편집기와 overlay의 상단 inset/padding은 0으로 유지하여 첫 줄이 문서 슬롯 상단에 붙는다.
- gutter: 거터는 본문과 같은 `Flickable` 콘텐츠 안에 배치하며, 논리 줄마다 번호 하나를 표시한다. 논리 줄
  분할과 row y/height 생성은 C++ `ContentsLineNumberRailMetrics`가 담당한다. wrap은 번호를 늘리지 않고
  해당 번호 행의 높이만 실제 본문 표시 높이만큼 커진다.
- resource frame: 리소스 프레임은 본문 텍스트 컬럼 폭의 100%로 렌더한다.
- minimap: 미니맵 행 수는 parser 논리 줄 수가 아니라 `editorVisualLineCount`로 전달되는 실제 에디터 wrap 결과 줄 수를 따른다. 리소스 프레임처럼 별도 높이를 차지하는 블록은 표시 높이를 본문 줄 높이로 나눈 줄 수만큼 미니맵에 반영한다.
- minimap width: 미니맵 각 행의 폭은 `editorVisualLineWidthRatios`로 전달되는 실제 표시 줄 길이를 따른다. 텍스트 행은 본문에서 보이는 길이에 비례하고, 리소스 프레임 높이에서 파생된 행은 프레임이 본문 폭을 채우므로 full width로 둔다.
- minimap drag: 미니맵은 세로 드래그를 정규화된 scroll ratio로 내보내고, `ContentsDisplayView.qml`이 이를 본문 `Flickable.contentY`로 변환한다.
- scrolling: 본문 중앙 슬롯은 `Flickable` viewport를 소유하며 긴 노트 본문은 이 viewport 안에서 세로 스크롤된다.
- pointer: 렌더 overlay가 꺼져 있으면 `LV.TextEditor`의 OS/Qt 선택 경로를 그대로 사용한다. 렌더 overlay가
  보이면 숨겨진 RAW 태그가 투명 텍스트 지오메트리를 왜곡하므로, 마우스 드래그 좌표를 `displayGeometryText`
  logical 좌표로 읽고 `logicalToSourceOffsets`로 RAW selection range를 복원한다. 이 pointer bridge는 IME
  composition 중에는 비활성화된다.
- cursor: 렌더 overlay가 보이는 동안 네이티브 커서가 숨겨진 inline formatting 태그 내부로 들어가면 이동
  방향에 맞춰 태그 앞/뒤의 안전한 RAW 경계로 보정한다.
- programmatic sync: 포커스된 native selection 중에는 inline editor controller가 host-side 텍스트 복원을
  defer/reject할 수 있어 selection이 즉시 해제되지 않는다.
- tag insertion: 명시적 포맷팅 및 본문 태그 단축키는 C++ tag insertion controller가 만든 RAW payload를
  적용한 뒤 일반 `sourceTextEdited` 경로로 저장한다. payload 생성은 live editor buffer를 기준으로 하며,
  하이라이트는 `Cmd/Ctrl+Shift+E`를 사용한다.
- 변경 시: 위 영어 본문을 수정하면 이 한국어 하단 섹션도 함께 최신 상태로 맞춘다.
