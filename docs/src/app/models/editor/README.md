# `src/app/models/editor`

## Scope
- Mirrored source directory: `src/app/models/editor`
- Child directories: 15
- Child files: 0

## Child Directories
- `bridge`
- `diagnostics`
- `display`
- `format`
- `gutter`
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

## Child Files
- No direct source files.

## Intended Detailed Sections
- Module responsibilities and architectural layer
- Internal submodule boundaries
- Cross-directory dependencies
- Runtime ownership and lifecycle rules
- Testing strategy and coverage map
- Known hotspots and refactor priorities

## Current Notes
- `parser/ContentsWsnBodyBlockParser.*` is now the first dedicated `.wsnbody` block-parser layer under
  `src/app/models/editor`.
  It prefers an iiXml document tree for top-level semantic body tags, emits one ordered `renderedDocumentBlocks`
  projection, and keeps the legacy `renderedAgendas` / `renderedCallouts` side payloads only as compatibility views
  over that same parse pass.
- `renderer/ContentsHtmlBlockRenderPipeline.*` is now the explicit RAW editor render-pipeline stage that sits between
  parser output and final RichText/QML consumption.
  It converts parser-owned blocks into HTML tokens, runs the token projection through iiXml + iiHtmlBlock, and
  normalizes the resulting display-block objects into stable HTML blocks before the live editor paints them.
- `projection/ContentsEditorPresentationProjection.*` republishes that renderer output to QML, including
  `htmlTokens` and `normalizedHtmlBlocks`, so the final editor host consumes block metadata instead of rediscovering
  block structure in JavaScript.
- `format/ContentsTextFormatRenderer.*` is the editor-owned document render backend for note-body HTML projection.
  It no longer lives under the paper display model because bold, italic, highlight, and related tags are editor body
  responsibilities, not page-surface responsibilities.
- `format/ContentsInlineStyleOverlayRenderer.*` isolates block-local inline-style overlay rendering from the broader
  document renderer contract.
- `format/ContentsPlainTextSourceMutator.*` owns plain-text RAW span replacement so ordinary typing no longer calls a
  renderer object to rewrite source.
- `format/ContentsRawInlineStyleMutationSupport.js` owns selection-based inline-style RAW mutations.
  Formatting commands now splice opening/closing inline tags directly around the resolved RAW selection span in
  `.wsnbody` without routing through a formatter QObject.
- `tags/ContentsRawBodyTagMutationSupport.js` is the editor-owned RAW body-tag mutation helper for generated agenda,
  selected-range callout wrapping, callout insertion, break, and generic raw tag text. Input events may request tags,
  but the helper builds the next-source payload directly from `.wsnbody` string splices without routing through an
  extra QObject planner.
- `structure/ContentsStructuredDocument*` owns the structured document host, collection policy, focus policy, mutation
  policy, and blocks model used by `ContentsStructuredDocumentFlow.qml`.
- `gutter/ContentsGutterLayoutMetrics.*` owns gutter width, line-number count, inactive-line sentinel, and
  line-number column metric calculations that used to live in QML.
- `gutter/ContentsGutterLineNumberGeometry.*` owns gutter line-number y-coordinate projection from the live editor
  geometry, with deterministic fallback entries for standalone frames.
- `gutter/ContentsGutterMarkerGeometry.*` owns gutter marker projection for the live cursor line and current RAW lines
  that differ from the saved `.wsnbody` snapshot.
- `minimap/ContentsMinimapLayoutMetrics.*` owns minimap width, row-count, and visibility-to-width calculations that
  used to live in QML.
- Gutter and minimap model code must remain direct editor-domain chrome model slices under
  `src/app/models/editor/gutter` and `src/app/models/editor/minimap`. Do not reintroduce `src/app/models/gutter`,
  `src/app/models/minimap`, or nested `src/app/models/editor/display/gutter|minimap` directories.
- `display/ContentsDisplay*` owns editor-host display coordination for selection, context menus, mount plans, refresh
  plans, and viewport math.
- Local RAW editor-source writes now converge through
  `session/ContentsEditorSessionController::commitRawEditorTextMutation(...)`, so QML controllers can propose next
  `.wsnbody` text without owning `editorText` writes, local-authority marking, or persistence scheduling.
- Editor-domain support JavaScript remains in the owning editor subdirectory when a small read-side QML helper is
  already part of the architecture, but QML view files stay under `src/app/qml/view/contents/editor`.
- Semantic text blocks such as `paragraph`, `title`, `subTitle`, and `eventDescription` now keep two coordinate
  systems in that parser contract:
  - wrapper spans (`blockSourceStart` / `blockSourceEnd`, open/close tag offsets) preserve the authored outer tag
  - editable spans (`sourceStart` / `sourceEnd` / `sourceText`) point only at the inner text content so the
    structured paragraph editor can rewrite RAW without stripping the outer semantic tag
- The current UI-thread hotspot priority is note-open behavior, not steady-state typing.
- Note-open reconcile and structured-tag correction now route file I/O through worker-thread queues before the result
  is mirrored back onto the main thread.
- Structured-flow block edits now also stop short of rebuilding the legacy full-document editor presentation on every
  keystroke, and `ContentsDisplayView.qml` no longer mounts the unreachable whole-note inline editor path.
- Structured-flow focus restoration now targets one reparsed block per request instead of fanning out through the whole
  document tree, which reduces per-mutation main-thread work on longer structured notes.
- The remaining focus path in `ContentsStructuredDocumentFlow.qml` is now also tokenless and request-driven, trimming
  the extra watcher/state churn that used to sit around that one-block restore path.
- Paper/page/print-specific display helpers remain in `src/app/models/display/paper`, while editor-body formatting,
  display coordination, and structured document behavior now stay under the editor domain.
- The note editor keeps document-source selection and viewport/minimap math under `src/app/models/editor/display`
  instead of leaving those responsibilities embedded in `ContentsDisplayView.qml`. The QML host still wires live state,
  but same-note source resolution, line-offset lookup, and minimap viewport math now execute through dedicated C++
  editor-domain objects.
- Editor snapshot reconcile and correction-complete paths now also avoid overlapping same-note fetches and duplicate
  post-sync UI refresh scheduling, further reducing repeated main-thread work after note open and background sync.
- Projects note projection now exposes the same lightweight persisted-body apply path as the other
  note-list-backed hierarchies, so validator-applied structured corrections no longer force an extra immediate
  metadata reload when the selected note lives under Projects.
- Agenda/callout edge handling is now tighter:
  - empty middle agenda tasks no longer truncate later siblings on Enter
  - entity-only agenda task bodies no longer count as blank
  - callout cursor-only focus restores now keep active-block tracking in sync
- Structured note-open now no longer keeps agenda/callout render projection entirely on the UI thread. The editor host
  keeps structured flow mounted during a worker-thread render pass so note entry does not block on synchronous
  structured parsing.
- Editor-area regression coverage now lives in the shared `test/cpp` suite rather than in a standalone editor-only
  target, so source/docs for this directory must stay aligned with the main CTest gate.

## 한국어

이 섹션은 위 README 내용을 한국어로 확인하기 위한 하단 요약이다.

- 대상: ``src/app/models/editor`` (`docs/src/app/models/editor/README.md`)
- 위치: `docs/src/app/models/editor`
- 역할: 이 파일은 해당 디렉터리나 모듈의 구조, 책임, 운영 규칙, 검증 기준을 설명한다. 거터와 미니맵 계산은 각각 `gutter`, `minimap` 하위 모델이 담당하며 커서/미저장 줄 막대 계산도 `gutter` 모델이 담당한다. `src/app/models/gutter`, `src/app/models/minimap`, `src/app/models/editor/display/gutter|minimap`는 재도입하지 않는다.
- 기준: 파일 경로, 명령, API 이름, 세부 변경 이력은 위 영어 본문을 원문 기준으로 유지한다.
- 변경 시: 위 영어 본문을 수정하면 이 한국어 하단 섹션도 함께 최신 상태로 맞춘다.
