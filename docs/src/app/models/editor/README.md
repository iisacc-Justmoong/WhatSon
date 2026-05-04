# `src/app/models/editor`

## Scope
- Mirrored source directory: `src/app/models/editor`
- Child files: 0

## Child Directories
- `bridge`
- `diagnostics`
- `display`
- `format`
- `input`
- `minimap`
- `parser`
- `persistence`
- `projection`
- `renderer`
- `resource`
- `session`
- `structure`
- `tags`
- `text`

## Current Notes
- `parser/ContentsWsnBodyBlockParser.*` is the dedicated `.wsnbody` block-parser layer. It prefers an iiXml document
  tree for top-level semantic body tags, emits one ordered `renderedDocumentBlocks` projection, and keeps the legacy
  `renderedAgendas` / `renderedCallouts` side payloads only as compatibility views over that same parse pass.
- `renderer/ContentsHtmlBlockRenderPipeline.*` is the explicit RAW editor render-pipeline stage that sits between
  parser output and final RichText/QML consumption.
- `projection/ContentsEditorPresentationProjection.*` republishes renderer output to QML, including `htmlTokens` and
  `normalizedHtmlBlocks`, so the final editor host consumes block metadata instead of rediscovering block structure in
  JavaScript.
- `format/ContentsTextFormatRenderer.*` is the editor-owned document render backend for note-body HTML projection.
- `format/ContentsInlineStyleOverlayRenderer.*` isolates block-local inline-style overlay rendering from the broader
  document renderer contract.
- `format/ContentsPlainTextSourceMutator.*` owns plain-text RAW span replacement so ordinary typing no longer calls a
  renderer object to rewrite source.
- `tags/ContentsEditorTagInsertionController.*` owns RAW tag insertion payloads for formatting tags such as `<bold>`
  and `<italic>`, generated body tags such as `<agenda>`, `<callout>`, and `<break>`, and selected-range body tag
  wrapping.
- `structure/ContentsStructuredDocument*` owns the structured document host, collection policy, focus policy, mutation
  policy, and blocks model used by `ContentsStructuredDocumentFlow.qml`.
- `minimap/ContentsMinimapLayoutMetrics.*` owns minimap width, row-count, and visibility-to-width calculations that
  used to live in QML.
- Minimap model code must remain a direct editor-domain chrome model slice under `src/app/models/editor/minimap`.
  Do not reintroduce `src/app/models/minimap` or nested `src/app/models/editor/display/minimap` directories.
- `display/ContentsDisplay*` owns editor-host display coordination for selection, context menus, mount plans, refresh
  plans, and viewport math.
- Local RAW editor-source writes converge through
  `session/ContentsEditorSessionController::commitRawEditorTextMutation(...)`, so QML controllers can propose next
  `.wsnbody` text without owning `editorText` writes, local-authority marking, or persistence scheduling.
- Semantic text blocks such as `paragraph`, `title`, `subTitle`, and `eventDescription` keep two coordinate systems:
  wrapper spans (`blockSourceStart` / `blockSourceEnd`, open/close tag offsets) preserve the authored outer tag, while
  editable spans (`sourceStart` / `sourceEnd` / `sourceText`) point only at the inner text content.
- Editor-area regression coverage lives in the shared `test/cpp` suite, so source/docs for this directory must stay
  aligned with the main CTest gate.

## 한국어

이 섹션은 위 README 내용을 한국어로 확인하기 위한 하단 요약이다.

- 대상: ``src/app/models/editor`` (`docs/src/app/models/editor/README.md`)
- 위치: `docs/src/app/models/editor`
- 역할: 이 파일은 해당 디렉터리나 모듈의 구조, 책임, 운영 규칙, 검증 기준을 설명한다. 미니맵 계산은 `minimap` 하위 모델이 담당한다. `src/app/models/minimap`, `src/app/models/editor/display/minimap`는 재도입하지 않는다.
- 기준: 파일 경로, 명령, API 이름, 세부 변경 이력은 위 영어 본문을 원문 기준으로 유지한다.
- 변경 시: 위 영어 본문을 수정하면 이 한국어 하단 섹션도 함께 최신 상태로 맞춘다.
