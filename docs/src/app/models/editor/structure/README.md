# `src/app/models/editor/structure`

## Responsibility
Owns the structured note document host and policies used by the editor body.

## Current Modules
- `ContentsStructuredDocumentBlocksModel.*`
  Publishes the ordered structured block stream to QML without resetting stable suffix rows.
- `ContentsStructuredDocumentCollectionPolicy.*`
  Normalizes parser/QML block collections and resource entries.
- `ContentsStructuredDocumentFocusPolicy.*`
  Computes focus restoration requests for structured blocks.
- `ContentsStructuredDocumentHost.*`
  Holds active structured-document state and publishes selection-clear revisions.
- `ContentsStructuredDocumentMutationPolicy.*`
  Builds RAW source insertion, deletion, merge, and split payloads for structured block edits.
  Resource insertion payloads always leave the caret on an editable source boundary after the inserted resource tag;
  EOF image paste therefore materializes a trailing empty text block instead of focusing the atomic resource card.
  Empty-block backward deletion also uses this policy to remove a preceding self-closing `<resource ... />` tag from
  RAW while preserving the current empty paragraph as the editable focus target.
- `ContentsLogicalLineLayoutSupport.js`
  Maps live `TextEdit` geometry into structured block logical-line entries.
- `ContentsStructuredCursorSupport.js`
  Converts plain-text cursor positions to RAW source offsets for structured agenda, callout, and semantic text blocks.

## Boundary
- These types are editor-domain structure helpers because they coordinate ordinary note body blocks.
- Parser output remains in `src/app/models/editor/parser`; HTML rendering remains in `src/app/models/editor/renderer`.

## 한국어

이 섹션은 위 README 내용을 한국어로 확인하기 위한 하단 요약이다.

- 대상: ``src/app/models/editor/structure`` (`docs/src/app/models/editor/structure/README.md`)
- 위치: `docs/src/app/models/editor/structure`
- 역할: 이 파일은 해당 디렉터리나 모듈의 구조, 책임, 운영 규칙, 검증 기준을 설명한다.
- 기준: 파일 경로, 명령, API 이름, 세부 변경 이력은 위 영어 본문을 원문 기준으로 유지한다.
- 변경 시: 위 영어 본문을 수정하면 이 한국어 하단 섹션도 함께 최신 상태로 맞춘다.
