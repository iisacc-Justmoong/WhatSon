# `src/app/models/editor/parser`

## Status
- Directory mirror generated from the current `src` tree.
- This file is the entry point for the detailed documentation pass of this directory.

## Scope
- Mirrored source directory: `src/app/models/editor/parser`
- Child directories: 0
- Child files: 2

## Child Directories
- No child directories.

## Child Files
- `ContentsWsnBodyBlockParser.cpp`
- `ContentsWsnBodyBlockParser.hpp`

## Current Notes
- `ContentsWsnBodyBlockParser` is the new editor-side read parser for `.wsnbody`.
- It now prefers an `iiXml::Parser::TagParser` document tree for supported explicit body tags, then emits one ordered
  `renderedDocumentBlocks` list for the structured QML editor host.
- The older token scan is retained only as a recovery lane for transient malformed edit states that cannot yet produce an
  iiXml tree.
- Agenda/callout payloads are now also derived in that same pass instead of being reparsed by independent read-side
  backends before the renderer can merge them.
- Explicit semantic text blocks now expose:
  - wrapper geometry through `blockSourceStart` / `blockSourceEnd` plus open/close-tag offsets
  - editable content geometry through `sourceStart` / `sourceEnd` / `sourceText`
  This keeps `ContentsDocumentTextBlock.qml` on an inner-content editing contract even when the authored RAW still
  uses wrapper tags such as `<paragraph>...</paragraph>`.
- Every emitted block now also carries one generic document-block trait payload for the QML flow host:
  `plainText`, `textEditable`, `atomicBlock`, `logicalLineCountHint`, `minimapVisualKind`, and
  `minimapRepresentativeCharCount`.
- The repository still does not maintain an in-repo automated editor test suite, so parser regression expectations are
  documented here rather than enforced by local tests.

## 한국어

이 섹션은 위 README 내용을 한국어로 확인하기 위한 하단 요약이다.

- 대상: ``src/app/models/editor/parser`` (`docs/src/app/models/editor/parser/README.md`)
- 위치: `docs/src/app/models/editor/parser`
- 역할: 이 파일은 해당 디렉터리나 모듈의 구조, 책임, 운영 규칙, 검증 기준을 설명한다.
- 기준: 파일 경로, 명령, API 이름, 세부 변경 이력은 위 영어 본문을 원문 기준으로 유지한다.
- 변경 시: 위 영어 본문을 수정하면 이 한국어 하단 섹션도 함께 최신 상태로 맞춘다.
