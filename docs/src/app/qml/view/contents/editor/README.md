# `src/app/qml/view/contents/editor`

## Scope

Editor-facing QML view components for the center content surface.

## Child Files

- `ContentsDisplaySurfaceHost.qml`
- `ContentsDisplayView.qml`
- `ContentsInlineFormatEditor.qml`
- `ContentsResourceEditorView.qml`
- `ContentsResourceViewer.qml`
- `ContentsStructuredDocumentFlow.qml`

## Current Pipeline

- `ContentsDisplayView.qml` mounts the selected note RAW body into `ContentsEditorSessionController`.
- `ContentsEditorPresentationProjection` converts that RAW body into editor HTML plus renderer-owned block metadata.
- `ContentsMinimapLayoutMetrics` resolves minimap chrome metrics under `src/app/models/editor/minimap`.
- `ContentsDisplayView.qml` owns the visible editor chrome layout and composes the parent `view/contents` minimap with
  `ContentsStructuredDocumentFlow.qml` in one `LV.HStack`.
- `ContentsStructuredDocumentFlow.qml` consumes `editorSurfaceHtml`, `logicalText`, `htmlTokens`, and
  `normalizedHtmlBlocks`, and passes `logicalText` plus the logical cursor position to the inline editor's geometry
  probe so the overlay cursor measures visible text instead of RichText HTML tag positions. The same renderer-owned
  `normalizedHtmlBlocks` stream is forwarded as selection metadata so resource selection uses the iiHtmlBlock display
  block span as one atomic block. It also binds explicit formatting and body-tag shortcuts to the C++ tag insertion
  controller so `Cmd/Ctrl+B`, `I`, `U`, `H`, and body commands such as callout/agenda insert or wrap proprietary RAW
  tags before the normal session persistence path runs.
- The center document slot owns a vertical `Flickable` viewport around `ContentsStructuredDocumentFlow.qml`; long note
  bodies scroll inside that viewport.
- `ContentsInlineFormatEditor.qml` keeps editing on an `LV.TextEditor` plain-text buffer while displaying the read-side
  RichText overlay. When native selection is active, that overlay stays visible so WYSIWYG text and resource frames do
  not collapse back to RAW tags; the underlying editor still owns the source range and keeps native selection highlight
  visible while RAW selected glyphs stay transparent. The rendered overlay mirrors text selection in logical
  coordinates, and resource spans from
  iiHtmlBlock metadata paint as one atomic block-level selection rectangle. Resource-backed overlays also stay pinned
  during ordinary native typing/composition turns, so a refresh gap cannot reveal the RAW `<resource ... />` tag in
  place of the frame. The editor paints an explicit blinking cursor above the overlay while the underlying RAW text and
  native RAW cursor delegate are hidden. The rendered surface is stacked below the transparent native editor and does
  not install pointer handlers above it; the mounted `LV.TextEditor` also opts into native gesture handling so clicks,
  drags, and modifier-based selection stay on the native text-edit path. Programmatic text replacement is routed through
  the inline editor controller so focused native selection can defer host-side surface refresh. Inline resource frames
  are rendered at 100% of the editor text-column width. The editor body has no top inset or overlay padding, so the
  first text line starts at the top of the document slot.
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
  소유하고 native selection highlight는 그대로 표시한다. 단 RAW selected glyph는 투명하게 유지한다. 렌더된
  텍스트 선택은 overlay의 logical selection으로 동기화하고, 리소스는 iiHtmlBlock display block 하나로 판정해
  atomic block 선택 rect 하나만 표시한다.
- cursor: RichText overlay가 보이는 동안에는 논리 텍스트 좌표 기반의 표면 커서만 overlay 위에 그리고, 아래의
  네이티브 RAW 커서 delegate는 숨긴다.
- top inset: 본문 편집기와 overlay의 상단 inset/padding은 0으로 유지하여 첫 줄이 문서 슬롯 상단에 붙는다.
- resource frame: 리소스 프레임은 본문 텍스트 컬럼 폭의 100%로 렌더한다.
- scrolling: 본문 중앙 슬롯은 `Flickable` viewport를 소유하며 긴 노트 본문은 이 viewport 안에서 세로 스크롤된다.
- pointer: 렌더 표면은 native editor 아래에 있으며 별도 포인터 핸들러를 두지 않는다. `LV.TextEditor`도 native
  gesture 경로를 사용하게 하여 OS/Qt 선택 경로를 그대로 사용한다.
- programmatic sync: 포커스된 native selection 중에는 inline editor controller가 host-side 텍스트 복원을
  defer/reject할 수 있어 selection이 즉시 해제되지 않는다.
- tag insertion: 명시적 포맷팅 및 본문 태그 단축키는 C++ tag insertion controller가 만든 RAW payload를
  적용한 뒤 일반 `sourceTextEdited` 경로로 저장한다.
- 변경 시: 위 영어 본문을 수정하면 이 한국어 하단 섹션도 함께 최신 상태로 맞춘다.
