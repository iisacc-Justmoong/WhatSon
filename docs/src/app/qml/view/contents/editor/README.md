# `src/app/qml/view/contents/editor`

## Scope

Editor-facing QML view components for the center content surface.

## Child Files

- `ContentsInlineFormatEditor.qml`
- `ContentsLineNumberRail.qml`
- `ContentsResourceEditorView.qml`
- `ContentsResourceViewer.qml`
- `ContentsStructuredDocumentFlow.qml`

## Current Pipeline

- `ContentViewLayout.qml` composes the live note editor chrome directly and instantiates
  `ContentsEditorDisplayBackend` from `src/app/models/editor/display`.
- `ContentsEditorDisplayBackend` mounts the selected note RAW body into `ContentsEditorSessionController` and owns the
  display-side session/projection/resource/minimap model objects.
- `ContentsEditorPresentationProjection` converts that RAW body into editor HTML plus renderer-owned block metadata.
- `ContentsEditorGeometryProvider` measures visible editor geometry for chrome snapshots,
  `ContentsEditorVisualLineMetrics` normalizes measured visual-line snapshots for the minimap, and
  `ContentsMinimapLayoutMetrics` resolves minimap chrome metrics under `src/app/models/editor/minimap`.
- `ContentViewLayout.qml` owns only visible editor chrome layout: one `LV.HStack`, a vertical `Flickable`,
  `ContentsLineNumberRail.qml`, `ContentsStructuredDocumentFlow.qml`, and the parent `view/contents` minimap with
  `LV.Theme.gap8` top and bottom chrome padding.
- `ContentViewLayout.qml` mounts `ContentsLineNumberRail.qml` inside the same scrollable document content item as
  `ContentsStructuredDocumentFlow.qml`, so gutter rows and editor body rows share one y-coordinate system.
- `ContentsStructuredDocumentFlow.qml` consumes `editorSurfaceHtml`, `logicalText`, `projectionSourceText`,
  `htmlTokens`, and `normalizedHtmlBlocks`, and passes projection-ready logical display text to the inline editor's
  geometry probe so pointer hit testing measures visible text instead of RichText HTML tag positions or RAW `.wsnbody`
  tag bytes. When the projection source snapshot temporarily lags behind the session
  source, the flow keeps the last ready logical text and editor-surface HTML instead of falling back to RAW source or
  plain logical text. The same renderer-owned `normalizedHtmlBlocks` stream is forwarded as selection metadata so
  resource selection mapping can treat the iiHtmlBlock display block span as one atomic block without painting a second
  selection layer. It also binds explicit formatting and body-tag shortcuts to the C++ tag insertion
  controller so `Cmd/Ctrl+B/I/U`, `Cmd/Ctrl+Shift+E` for highlight, and body commands such as callout/agenda insert or
  wrap proprietary RAW tags before the normal session persistence path runs. These commands read the live inline editor
  buffer when constructing payloads so consecutive tag commands do not combine fresh selection offsets with stale parent
  `sourceText` bindings.
- The center document slot owns a vertical `Flickable` viewport around `ContentsStructuredDocumentFlow.qml`; long note
  bodies scroll inside that viewport. The viewport may be reset only for actual note identity changes requested by
  `ContentsEditorDisplayBackend`; typing and same-note projection refreshes preserve the current `contentY`. The inline
  editor also restores the legacy `LV.Theme.gap16` bottom inset by adding it to the reported scrollable content height
  after the measured rendered body height, so the final body line is not pinned to the viewport edge.
- The gutter row list is driven by `ContentsStructuredDocumentFlow.editorLogicalGutterRows`, which is resolved by the
  C++ `ContentsLineNumberRailMetrics` object mounted inside `ContentsInlineFormatEditor.qml`. The metrics object
  receives measured row-geometry snapshots from `ContentsEditorGeometryProvider` instead of direct TextEdit, cursor,
  selection, or resource overlay references. Each gutter row represents one full-document logical text line, including
  blank lines introduced between hidden RAW tags. Wrapped text does not increment the number count, but the row height
  is measured from the live editor geometry so a long paragraph that wraps onto three visible lines gives one numbered
  gutter row with three visible lines of height. The rail width comes from `ContentsLineNumberRail.preferredWidth`,
  which halves the leading blank area before the number column. The host also forwards RAW cursor/selection offsets so
  the rail can paint a blue active-line bar on the cursor row or selected rows. Atomic resource frames count as one
  logical row with one gutter-line height. Line-number y snapshots come from the plain logical display probe so ordinary
  text rows keep independent positions; in rendered mode the rendered HTML resource image height supplies the
  visual-height delta of an atomic resource frame, which places later rows below the frame without converting the frame
  into extra gutter rows or adopting RichText row coordinates for unrelated text.
  Inline resource HTML suppresses paragraph line-height around the generated frame image, so the next gutter row can
  use the encoded frame image height as the actual visual bottom anchor. The geometry adapter compares that frame
  height to the next plain logical row's base y, not to the hidden placeholder line-box height. If QML asks before the
  next plain row has a measurable y, the adapter uses the full frame height so the first post-resource row still lands
  on the frame bottom. If multiple logical rows are clamped out of the same resource frame, the adapter advances each
  following row by its published height so line 2 and a blank line 3 cannot share the same gutter y coordinate.
  `ContentsInlineFormatEditor.qml` also exposes read-only geometry-provider snapshots so tests can distinguish raw
  measured rows from the final one-line resource gutter rows.
- The minimap row count is driven by `ContentsStructuredDocumentFlow.editorVisualLineCount`, which comes from measured
  visual-line snapshots normalized by the C++ `ContentsEditorVisualLineMetrics` object. A single source tag or
  paragraph that wraps onto two visible editor lines therefore produces two minimap rows. Tall rendered blocks such as
  resource frames use their visible content height divided by the editor line height, so their minimap footprint
  follows the amount of vertical space they occupy.
- The minimap row widths are driven by `ContentsStructuredDocumentFlow.editorVisualLineWidthRatios`. Text rows use the
  visible line length measured by `ContentsEditorGeometryProvider` and normalized by `ContentsEditorVisualLineMetrics`;
  height-derived rows that cannot be probed from text geometry remain full width inside the minimap's padded rail.
- The minimap can be dragged vertically without rendering scrollbar chrome. `Minimap.qml` emits direct drag pixel
  deltas from its invisible drag surface and `ContentViewLayout.qml` applies those deltas to the center editor
  `Flickable.contentY` without smoothing or ratio scaling.
- `ContentsInlineFormatEditor.qml` keeps editing on an `LV.TextEditor` plain-text surface while displaying the read-side
  RichText overlay. In rendered mode that native surface contains the visible logical projection, not RAW tag bytes;
  visible text deltas are converted back to RAW `.wsnbody` splices by `ContentsWysiwygEditorPolicy`. When native
  selection is active, that overlay stays visible so WYSIWYG text and resource frames do not collapse back to RAW tags;
  the underlying editor still owns the visible logical range, while exported cursor/selection properties map that range
  back to RAW source offsets for persistence, gutter state, and tag commands. Native selection highlight stays visible
  for ranges that contain visible logical content while the underlying logical glyphs stay transparent. Tag-only source
  ranges, such as empty formatting wrappers, do not paint native selection because those tags produce no
  visible editor content, and restoring such a range keeps it collapsed so the next visible character is not selected.
  The rendered overlay no longer mirrors text selection or paints resource selection rectangles; visual selection is
  the same native `LV.TextEditor` selection that owns editing. Rendered overlays stay pinned during ordinary native
  typing/composition turns, so a refresh gap cannot reveal plain logical text or the RAW `<resource ... />` tag in
  place of formatted text and resource frames. The rendered surface is stacked below the transparent native editor.
  While that
  overlay is visible, a thin pointer bridge maps mouse-drag hit testing through a transparent plain-text geometry probe
  whose text is exactly `displayGeometryText`, then uses the C++ `coordinateMapper` to restore the matching RAW source
  selection on `LV.TextEditor`; this keeps sub-word selection precise even when hidden source tags or RichText document
  positions would otherwise distort the coordinate space. That
  visible-to-RAW selection policy is implemented by C++ `ContentsWysiwygEditorPolicy`; QML only applies the returned
  range to the live `LV.TextEditor`. If the pointer lands inside a measured atomic resource frame, the bridge snaps to
  that resource's single U+FFFC logical placeholder and selects the resource tag as one block instead of asking
  `positionAt(...)` for fake internal text positions. Collapsed cursor movement is also excluded from that placeholder:
  when the native logical cursor lands on an atomic resource line, C++ policy moves it to the nearest prose boundary
  outside the frame, falling back to the opposite side if the preferred boundary is still on the resource row; for
  resource-only content where no text boundary exists, it hides the caret. Collapsed mouse clicks use the same bridge
  for cursor placement by restoring the live `LV.TextEditor` cursor/selection directly. The
  rendered overlay does not mount a second projected cursor object; the visible caret is the native editor cursor that
  receives typing and Backspace outside atomic resource frames. Non-empty drag selections keep the native selection
  model authoritative. The passive surface-selection editor mirrors the plain logical text only and does not accept
  focus, mouse selection, or key events. Double-click and
  triple-click gestures keep native-style line and paragraph selection by selecting the visible logical range before
  mapping it back to RAW source offsets. The bridge is inactive during IME composition. Plain-source and
  keyboard/modifier selection remain on the native text-edit path.
  If native cursor movement lands inside a hidden inline formatting tag, C++ policy returns the adjacent safe source
  boundary so arrow traversal skips zero-width tag bytes. Programmatic text replacement is routed through the inline
  editor controller, and the native `LV.TextEditor.text` property is not directly bound to the projection expression,
  so focused native edits, selections, and IME composition can defer host-side surface refresh. Inline
  resource frames are rendered at 100% of the editor text-column width. The editor body has no top inset or overlay
  padding, so the first text line starts at the top of the document slot.
- Resource-backed center-surface browsing is handled by `ContentsResourceEditorView.qml` and `ContentsResourceViewer.qml`.

QML in this directory must stay presentation-only. XML parsing, HTML tokenization, block object construction, WYSIWYG
source/selection policy, hidden-tag cursor normalization, and RAW source mutation policy remain in C++ model/renderer
objects.

## 한국어

이 섹션은 위 README 내용을 한국어로 확인하기 위한 하단 요약이다.

- 대상: ``src/app/qml/view/contents/editor`` (`docs/src/app/qml/view/contents/editor/README.md`)
- 위치: `docs/src/app/qml/view/contents/editor`
- 역할: 이 파일은 해당 디렉터리나 모듈의 구조, 책임, 운영 규칙, 검증 기준을 설명하며 본문 편집기는 `LV.TextEditor` 기준이다.
- 배치: 실제 노트 편집 화면은 `ContentViewLayout.qml`의 `LV.HStack`에서 본문 편집기와 미니맵 순서로 표시한다. 세션, projection, resource, minimap 계산은 `ContentsEditorDisplayBackend`가 담당한다.
- 기준: 파일 경로, 명령, API 이름, 세부 변경 이력은 위 영어 본문을 원문 기준으로 유지한다.
- selection: 본문 텍스트 선택 중에도 RichText overlay를 유지한다. 선택 source range는 `LV.TextEditor`가
  소유하는 visible logical selection에서 RAW offset으로 되돌려 계산한다. visible logical content가 있는 범위에
  대해서만 native selection highlight를 표시하고, underlying logical glyph는 투명하게 유지한다. 빈 formatting
  wrapper처럼 태그만 포함된 source range는 표시할 본문이 없으므로 native selection을 칠하지 않으며, 복원 시 다음
  visible character로 selection이 펼쳐지지 않게 collapsed caret로 유지한다. 렌더 overlay는 별도 logical
  selection이나 resource selection rect를 칠하지 않는다.
- cursor: RichText overlay가 보이는 동안에도 typing/Backspace를 받는 native `LV.TextEditor` cursor delegate 하나만
  보인다. 별도 projected cursor 객체는 두지 않는다.
- vertical inset: 본문 편집기와 overlay의 상단 inset/padding은 0으로 유지하여 첫 줄이 문서 슬롯 상단에 붙는다.
  하단은 기존 editor body breathing-room 계약대로 측정된 본문 높이 뒤에 `LV.Theme.gap16`을 더해 scrollable
  content extent에 포함한다.
- chrome padding: `ContentViewLayout.qml`의 editor chrome HStack은 상하 `LV.Theme.gap8` inset을 적용한다. 이
  padding은 거터, 본문 editor viewport, 미니맵 전체에 공통으로 적용되며, inline editor 내부 inset과는 분리된다.
- gutter: 거터는 본문과 같은 `Flickable` 콘텐츠 안에 배치하며, 전체 문서의 논리 줄마다 번호 하나를 표시한다.
  RAW 태그가 숨겨져도 태그 사이에 남는 빈 논리 줄은 거터 번호 슬롯을 유지한다. 논리 줄 분할과 row y/height
  생성은 C++ `ContentsLineNumberRailMetrics`가 담당한다. 지오메트리는
  `ContentsEditorGeometryProvider`가 만든 row snapshot으로만 들어오며, 거터 metrics는
  TextEdit/cursor/selection/resource overlay 객체에 직접 결합하지 않는다. wrap은 번호를 늘리지 않고 해당
  번호 행의 높이만 실제 본문 표시 높이만큼 커진다. 각 row의 y/height는 독립 snapshot으로 처리하며, 리소스
  프레임 row의 높이가 다음 줄 번호 위치를 연쇄적으로 밀지 않는다. 번호 왼쪽 빈 영역은 rail의 `preferredWidth` 계산으로
  기존 implicit blank의 절반만 사용한다. RAW cursor/selection offset은 host가 전달하며, 거터는 이를 row
  source range와 비교해 커서 줄 또는 선택된 줄에 파란색 active bar를 표시한다.
- resource frame: 리소스 프레임은 본문 텍스트 컬럼 폭의 100%로 렌더한다.
- minimap: 미니맵 행 수는 parser 논리 줄 수가 아니라 geometry adapter가 측정해 `editorVisualLineCount`로 전달되는 실제 에디터 wrap 결과 줄 수를 따른다. 리소스 프레임처럼 별도 높이를 차지하는 블록은 표시 높이를 본문 줄 높이로 나눈 줄 수만큼 미니맵에 반영한다.
- minimap width: 미니맵 각 행의 폭은 `editorVisualLineWidthRatios`로 전달되는 실제 표시 줄 길이를 따른다. 텍스트 행은 본문에서 보이는 길이에 비례하고, 리소스 프레임 높이에서 파생된 행은 프레임이 본문 폭을 채우므로 padded rail 내부에서 full width로 둔다. 미니맵 metrics는 측정된 ratio snapshot만 소비한다.
- minimap drag: 미니맵은 별도 스크롤바 chrome 없이 세로 드래그 pixel delta를 그대로 내보내고, `ContentViewLayout.qml`이 같은 delta를 본문 `Flickable.contentY`에 더한다.
- scrolling: 본문 중앙 슬롯은 `Flickable` viewport를 소유하며 긴 노트 본문은 이 viewport 안에서 세로 스크롤된다.
  실제 노트 id/path가 바뀌는 경우에만 backend reset 신호로 최상단 이동이 허용되며, 타이핑과 같은 노트의 projection
  refresh/reconcile은 현재 `contentY`를 보존해야 한다. scrollable content height는 inline editor가 보고하는
  `displayContentHeight`를 따른다. 이 값은 top inset 없이 `displayTextContentHeight + LV.Theme.gap16`이다.
- pointer: 렌더 overlay가 꺼져 있으면 `LV.TextEditor`의 OS/Qt 선택 경로를 그대로 사용한다. 렌더 overlay가
  보이면 숨겨진 RAW 태그와 RichText document position이 좌표계를 왜곡할 수 있으므로, 마우스 드래그 좌표를
  `displayGeometryText`와 1:1인 plain-text geometry probe로 읽고 C++ `ContentsWysiwygEditorPolicy`와
  `coordinateMapper`로 RAW selection range를 복원한다.
  QML은 반환된 range를 `LV.TextEditor`에 적용하는 뷰 역할만 수행한다. 드래그가 없는 클릭은 같은
  경로에서 live `LV.TextEditor` cursor placement로 처리한다. 별도 projected cursor 객체나 pointer cursor
  override는 두지 않는다. 드래그가 non-empty selection으로 확장되면 native selection 모델이 표시 상태를
  소유한다. 더블클릭은 표시 논리 줄, 세 번 클릭은 표시 문단을 선택한 뒤 RAW source offset으로 되돌린다. 이 pointer
  bridge는 IME composition 중에는 비활성화된다.
- cursor: 렌더 overlay가 보이는 동안 네이티브 커서는 visible logical text 좌표에서만 움직인다. QML은
  `logicalCursorPosition`, `sourceCursorPosition`, `sourceSelectionStart/End`를 분리해 노출하며, visible cursor는
  typing/Backspace를 받는 native `LV.TextEditor` cursor delegate 하나만 사용한다.
- programmatic sync: 포커스된 native selection 중에는 inline editor controller가 host-side 텍스트 복원을
  defer/reject할 수 있어 selection이 즉시 해제되지 않는다. native `LV.TextEditor.text`는 projection 표현식에
  직접 바인딩하지 않고 controller의 programmatic sync 경로로만 갱신한다.
- projection sync: `ContentsStructuredDocumentFlow.qml`은 `projectionSourceText`가 현재 `sourceText`와 일치할
  때의 `logicalText`와 `editorSurfaceHtml`만 표시 projection으로 채택한다. projection이 따라잡기 전에는 마지막
  정상 logical text와 RichText HTML을 유지하며 RAW `.wsnbody` source나 plain logical text로 fallback하지 않는다.
- tag insertion: 명시적 포맷팅 및 본문 태그 단축키는 C++ tag insertion controller가 만든 RAW payload를
  적용한 뒤 일반 `sourceTextEdited` 경로로 저장한다. payload 생성은 live editor buffer를 기준으로 하며,
  하이라이트는 `Cmd/Ctrl+Shift+E`를 사용한다.
- 변경 시: 위 영어 본문을 수정하면 이 한국어 하단 섹션도 함께 최신 상태로 맞춘다.
