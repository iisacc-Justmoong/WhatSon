# `src/app/models/editor`

## Status
- Directory mirror generated from the current `src` tree.
- This file is the entry point for the detailed documentation pass of this directory.

## Scope
- Mirrored source directory: `src/app/models/editor`
- Child directories: 3
- Child files: 0

## Child Directories
- `parser`
- `painter`
- `renderer`

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
- Paper/page/print-specific display helpers were extracted from `src/app/models/editor` into
  `src/app/models/display/paper` so the remaining editor domain stays focused on parsing and rendering mechanics rather
  than reusable view-mode state.
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
