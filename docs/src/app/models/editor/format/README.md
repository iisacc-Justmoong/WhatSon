# `src/app/models/editor/format`

## Responsibility
Owns editor-body inline formatting logic.

## Current Modules
- `ContentsCalloutHtmlRenderer.*`
  Owns shared transparent callout detection and emits the Qt RichText-compatible table frame, including the `3px`
  `bgcolor="#d9d9d9"` leading-bar cell.
- `ContentsInlineStyleOverlayRenderer.*`
  Renders block-local inline style overlays for structured text delegates without exposing the full document renderer
  contract.
- `ContentsPlainTextSourceMutator.*`
  Owns plain-text source-span replacement and bounded committed-URL canonicalization for ordinary typing paths. Very
  large replacement text is accepted without a synchronous web-link scan so paste latency stays tied to the RAW source
  mutation rather than link detection.
- `ContentsTextFormatRenderer.*`
  Converts RAW `.wsnbody` document snapshots into editor-surface HTML, preview HTML, and normalized render payloads.
- `ContentsTextHighlightRenderer.*`
  Keeps highlight alias and HTML style details out of the broader formatter implementation.

## Boundary
- This directory owns read-side rendering for body text formatting such as bold, italic, underline, strikethrough, and
  highlight.
- Formatting writes are editor tag insertions and live under `src/app/models/editor/tags` with callout/agenda paired
  tag insertion plus break and resource tag helpers.
- Paper and print geometry remain in `src/app/models/display/paper` and `src/app/models/display/paper/print`.
- The formatter may derive presentation from RAW source, but it must not own persisted formatting writes.

## 한국어

이 섹션은 위 README 내용을 한국어로 확인하기 위한 하단 요약이다.

- 대상: ``src/app/models/editor/format`` (`docs/src/app/models/editor/format/README.md`)
- 위치: `docs/src/app/models/editor/format`
- 역할: 이 파일은 해당 디렉터리나 모듈의 구조, 책임, 운영 규칙, 검증 기준을 설명하며, 대형 붙여넣기 입력은
  동기 URL canonicalization 때문에 막히면 안 된다는 기준도 포함한다. 콜아웃 렌더링은 전용 helper가 Qt RichText
  호환 table 구조와 `3px` 좌측 막대 셀을 소유한다.
- 기준: 파일 경로, 명령, API 이름, 세부 변경 이력은 위 영어 본문을 원문 기준으로 유지한다.
- 변경 시: 위 영어 본문을 수정하면 이 한국어 하단 섹션도 함께 최신 상태로 맞춘다.
