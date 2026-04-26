# `src/app/models/editor`

## Scope
- Mirrored source directory: `src/app/models/editor`
- Child directories: 13
- Child files: 0

## Child Directories
- `bridge`
- `diagnostics`
- `display`
- `format`
- `input`
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
  It tokenizes top-level semantic body tags once, emits one ordered `renderedDocumentBlocks` projection, and keeps the
  legacy `renderedAgendas` / `renderedCallouts` side payloads only as compatibility views over that same parse pass.
- `renderer/ContentsHtmlBlockRenderPipeline.*` is now the explicit RAW editor render-pipeline stage that sits between
  parser output and final RichText/QML consumption.
  It converts parser-owned blocks into HTML tokens, resolves per-token render strategy, and normalizes the result into
  stable HTML blocks before the live editor paints them.
- `format/ContentsTextFormatRenderer.*` is the editor-owned RAW inline-style projection and mutation backend.
  It no longer lives under the paper display model because bold, italic, highlight, and related tags are editor body
  responsibilities, not page-surface responsibilities.
- `structure/ContentsStructuredDocument*` owns the structured document host, collection policy, focus policy, mutation
  policy, and blocks model used by `ContentsStructuredDocumentFlow.qml`.
- `display/ContentsDisplay*` owns editor-host display coordination for selection, context menus, gutter/minimap state,
  mount plans, refresh plans, and viewport math.
- Editor-domain QML policy/controller/support files now also live under these responsibility directories. The
  `src/app/qml/view/content/editor` directory is reserved for view hosts, visual layers, and block delegates that match
  the Figma `ContentsView` / `ContentsDisplayView` surface.
- Semantic text blocks such as `paragraph`, `title`, `subTitle`, and `eventDescription` now keep two coordinate
  systems in that parser contract:
  - wrapper spans (`blockSourceStart` / `blockSourceEnd`, open/close tag offsets) preserve the authored outer tag
  - editable spans (`sourceStart` / `sourceEnd` / `sourceText`) point only at the inner text content so the
    structured paragraph editor can rewrite RAW without stripping the outer semantic tag
- The current UI-thread hotspot priority is note-open behavior, not steady-state typing.
- Note-open reconcile and structured-tag correction now route file I/O through worker-thread queues before the result
  is mirrored back onto the main thread.
- Structured-flow block edits now also stop short of rebuilding the legacy full-document editor presentation on every
  keystroke, and the fallback inline editor instance now unloads entirely while the note stays in structured mode.
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
