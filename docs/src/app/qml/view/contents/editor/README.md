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
- `ContentsGutterLayoutMetrics`, `ContentsGutterLineNumberGeometry`, `ContentsGutterMarkerGeometry`, and
  `ContentsMinimapLayoutMetrics` resolve gutter/minimap chrome metrics, gutter line-number y positions, and semantic
  cursor/unsaved marker positions under `src/app/models/editor/gutter` and
  `src/app/models/editor/minimap`.
- `ContentsDisplayView.qml` owns the visible editor chrome layout and composes the parent `view/contents` chrome
  components as `Gutter.qml -> ContentsStructuredDocumentFlow.qml -> Minimap.qml` in one `LV.HStack`.
  The gutter is transparent by default and must not visually separate itself from the editor body.
- `ContentsStructuredDocumentFlow.qml` consumes `editorSurfaceHtml`, `logicalText`, `htmlTokens`, and
  `normalizedHtmlBlocks`, and passes `logicalText` plus the logical cursor position to the inline editor's geometry
  probe so gutter labels and the overlay cursor measure visible text lines instead of RichText HTML tag positions.
  The same renderer-owned `normalizedHtmlBlocks` stream is forwarded as selection metadata so resource selection uses
  the iiHtmlBlock display block span as one atomic block.
- The center document slot owns a vertical `Flickable` viewport around `ContentsStructuredDocumentFlow.qml`; long note
  bodies scroll inside that viewport, and scroll movement refreshes gutter geometry against the same visible document
  coordinate system.
- `ContentsInlineFormatEditor.qml` keeps editing on an `LV.TextEditor` plain-text buffer while displaying the read-side
  RichText overlay. When native selection is active, that overlay stays visible so WYSIWYG text and resource frames do
  not collapse back to RAW tags; the underlying editor still owns the source range while RAW selection paint stays
  transparent. The rendered overlay mirrors text selection in logical coordinates, and resource spans from
  iiHtmlBlock metadata paint as one atomic block-level selection rectangle. The editor paints an explicit blinking
  cursor above the overlay while the underlying RAW text and native RAW cursor delegate are hidden. Clicks on the
  rendered surface are converted from visible logical text coordinates back to RAW source offsets before moving the
  editor cursor. The editor body has no top inset or overlay padding, so the first text line starts at the top of the
  document slot.
- Resource-backed center-surface browsing is handled by `ContentsResourceEditorView.qml` and `ContentsResourceViewer.qml`.

QML in this directory must stay presentation-only. XML parsing, HTML tokenization, block object construction, and RAW
source mutation policy remain in C++ model/renderer objects.

## 한국어

이 섹션은 위 README 내용을 한국어로 확인하기 위한 하단 요약이다.

- 대상: ``src/app/qml/view/contents/editor`` (`docs/src/app/qml/view/contents/editor/README.md`)
- 위치: `docs/src/app/qml/view/contents/editor`
- 역할: 이 파일은 해당 디렉터리나 모듈의 구조, 책임, 운영 규칙, 검증 기준을 설명하며 본문 편집기는 `LV.TextEditor` 기준이다.
- 배치: 실제 노트 편집 화면은 `ContentsDisplayView.qml`의 `LV.HStack`에서 거터, 본문 편집기, 미니맵 순서로 표시한다. 거터와 미니맵 계산, 거터 줄 번호 y 좌표 계산, 커서/미저장 줄 막대 계산은 C++ 모델 계층에서 수행한다.
- 기준: 파일 경로, 명령, API 이름, 세부 변경 이력은 위 영어 본문을 원문 기준으로 유지한다.
- selection: 본문 텍스트 선택 중에도 RichText overlay를 유지한다. 선택 source range는 `LV.TextEditor`가
  소유하되 RAW 선택 paint는 투명하게 유지한다. 렌더된 텍스트 선택은 overlay의 logical selection으로 동기화하고,
  리소스는 iiHtmlBlock display block 하나로 판정해 atomic block 선택 rect 하나만 표시한다.
- cursor: RichText overlay가 보이는 동안에는 논리 텍스트 좌표 기반의 표면 커서만 overlay 위에 그리고, 아래의
  네이티브 RAW 커서 delegate는 숨긴다.
- top inset: 본문 편집기와 overlay의 상단 inset/padding은 0으로 유지하여 첫 줄이 문서 슬롯 상단에 붙는다.
- gutter geometry: 거터 줄 번호는 RichText HTML 문자열이 아니라 보이는 논리 텍스트 probe의 line rectangle을 기준으로 삼고, 중복 y 좌표는 C++ 모델에서 겹치지 않게 보정한다.
- scrolling: 본문 중앙 슬롯은 `Flickable` viewport를 소유하며 긴 노트 본문은 이 viewport 안에서 세로 스크롤된다.
- cursor click: 렌더 표면 클릭은 보이는 logical text 좌표를 RAW offset table로 역변환해 커서를 이동한다.
- 변경 시: 위 영어 본문을 수정하면 이 한국어 하단 섹션도 함께 최신 상태로 맞춘다.
