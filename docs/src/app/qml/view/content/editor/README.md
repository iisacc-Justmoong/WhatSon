# `src/app/qml/view/content/editor`

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
- `ContentsDisplayView.qml` owns the visible editor chrome layout and composes
  `contents/Gutter.qml -> ContentsStructuredDocumentFlow.qml -> contents/Minimap.qml` in one `LV.HStack`.
  The gutter is transparent by default and must not visually separate itself from the editor body.
- `ContentsStructuredDocumentFlow.qml` consumes `editorSurfaceHtml`, `htmlTokens`, and `normalizedHtmlBlocks`.
- `ContentsInlineFormatEditor.qml` keeps editing on an `LV.TextEditor` plain-text buffer while displaying the read-side
  RichText overlay. When native selection is active, that overlay is hidden so the selected range is painted by the
  real editor text instead of a duplicate overlay layer. The editor body has no top inset or overlay padding, so the
  first text line starts at the top of the document slot.
- Resource-backed center-surface browsing is handled by `ContentsResourceEditorView.qml` and `ContentsResourceViewer.qml`.

QML in this directory must stay presentation-only. XML parsing, HTML tokenization, block object construction, and RAW
source mutation policy remain in C++ model/renderer objects.

## 한국어

이 섹션은 위 README 내용을 한국어로 확인하기 위한 하단 요약이다.

- 대상: ``src/app/qml/view/content/editor`` (`docs/src/app/qml/view/content/editor/README.md`)
- 위치: `docs/src/app/qml/view/content/editor`
- 역할: 이 파일은 해당 디렉터리나 모듈의 구조, 책임, 운영 규칙, 검증 기준을 설명하며 본문 편집기는 `LV.TextEditor` 기준이다.
- 배치: 실제 노트 편집 화면은 `ContentsDisplayView.qml`의 `LV.HStack`에서 거터, 본문 편집기, 미니맵 순서로 표시한다. 거터와 미니맵 계산, 거터 줄 번호 y 좌표 계산, 커서/미저장 줄 막대 계산은 C++ 모델 계층에서 수행한다.
- 기준: 파일 경로, 명령, API 이름, 세부 변경 이력은 위 영어 본문을 원문 기준으로 유지한다.
- selection: 본문 텍스트 선택 중에는 RichText overlay를 숨기고 `LV.TextEditor`의 실제 텍스트 선택 페인트를 사용한다.
- top inset: 본문 편집기와 overlay의 상단 inset/padding은 0으로 유지하여 첫 줄이 문서 슬롯 상단에 붙는다.
- 변경 시: 위 영어 본문을 수정하면 이 한국어 하단 섹션도 함께 최신 상태로 맞춘다.
