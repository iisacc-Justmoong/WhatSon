# `src/app/models/editor/renderer`

## Status
- Directory mirror generated from the current `src` tree.
- This file is the entry point for the detailed documentation pass of this directory.

## Scope
- Mirrored source directory: `src/app/models/editor/renderer`
- Child directories: 0
- Child files: 4

## Child Directories
- No child directories.

## Child Files
- `ContentsHtmlBlockRenderPipeline.cpp`
- `ContentsHtmlBlockRenderPipeline.hpp`
- `ContentsStructuredBlockRenderer.cpp`
- `ContentsStructuredBlockRenderer.hpp`

## Current Notes
- `ContentsHtmlBlockRenderPipeline.cpp` is now the explicit editor HTML pipeline:
  iiXml-backed parser blocks -> HTML tokens -> iiHtmlBlock block objects -> normalized HTML blocks -> final editor HTML
  document.
- `normalizedHtmlBlocks` now carry iiHtmlBlock-derived block-object metadata such as HTML block tag name, raw/value
  ranges, display-block state, and parser source.
- `ContentsStructuredBlockRenderer.cpp` now consumes `parser/ContentsWsnBodyBlockParser` as its single `.wsnbody`
  read-path source and republishes that parser result to QML.
- Agenda/callout compatibility lists still exist, but they are now side projections over the same unified parser pass
  rather than separate read-side backend merges.
- Shortcut/context-menu formatting no longer depends on a transient `QTextDocument` fragment merge to decide where a
  RAW source style starts or ends.
- Inline text formatting and highlight tag styling now live in `src/app/models/editor/format`; this renderer directory
  stays focused on block-level HTML and structured document projection.
- Page/print paper-surface helpers were moved out to `src/app/models/display/paper` and
  `src/app/models/display/paper/print`, leaving this directory focused on renderer-only concerns.
- `ContentsStructuredBlockRenderer.cpp` still emits verbose editor trace events for constructor/destructor turns,
  source binding changes, and projection refresh passes so the read-side renderer path can be traced alongside QML host
  updates.

## Intended Detailed Sections
- Module responsibilities and architectural layer
- Internal submodule boundaries
- Cross-directory dependencies
- Runtime ownership and lifecycle rules
- Testing strategy and coverage map
- Known hotspots and refactor priorities

## 한국어

이 섹션은 위 README 내용을 한국어로 확인하기 위한 하단 요약이다.

- 대상: ``src/app/models/editor/renderer`` (`docs/src/app/models/editor/renderer/README.md`)
- 위치: `docs/src/app/models/editor/renderer`
- 역할: 이 파일은 해당 디렉터리나 모듈의 구조, 책임, 운영 규칙, 검증 기준을 설명한다.
- 기준: 파일 경로, 명령, API 이름, 세부 변경 이력은 위 영어 본문을 원문 기준으로 유지한다.
- 변경 시: 위 영어 본문을 수정하면 이 한국어 하단 섹션도 함께 최신 상태로 맞춘다.
