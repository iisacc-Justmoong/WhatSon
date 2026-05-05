# `src/app/models/editor/tags`

## Responsibility
Owns editor tag-management helpers.

## Editor tag insertion
- Inline formatting tags such as `<bold>`, `<italic>`, `<underline>`, `<highlight>`, and their aliases.
- `<agenda>` / `<task>` parsing, task toggles, and agenda Enter behavior.
- `<callout>` parsing, selected-range wrapping, Shift+Enter body line breaks, and plain-Enter callout exit behavior.
- `<break>` canonicalization and structured verification.
- `<resource ... />` RAW source insertion and canonical resource-tag text generation.
- `ContentsEditorTagInsertionController` builds the common next-source RAW tag insertion payloads for inline
  formatting, generated agenda/callout/break insertion, and selected-range body tag wrapping. Formatting commands use
  this same RAW tag insertion model instead of a separate formatting mutation model.
- Inline formatting re-application normalizes the selected source span to existing inline-style tag boundaries before
  writing the next RAW snapshot, so WYSIWYG selections cannot split a hidden `<bold>` / `<highlight>` token and expose
  malformed source text.
- Structured body-tag linting and advisory correction state for parser/renderer feedback.

## Current Modules
- `ContentsAgendaBackend.*`
- `ContentsCalloutBackend.*`
- `ContentsEditorTagInsertionController.*`
- `ContentsStructuredTagValidator.*`
- `WhatSonStructuredTagLinter.*`
- `ContentsResourceTagTextGenerator.*`
- `ContentsResourceTagController.qml`

## Boundary
- Formatting rendering remains under `src/app/models/editor/format`, but formatting writes belong to this tag
  insertion boundary because RAW `.wsnbody` sees them as tag insertion.
- Re-formatting writes may use the parser/rendered projection contract to preserve balanced inline tags, but the
  rendered RichText/iiHtmlBlock output remains read-only presentation state and is never serialized as the source of
  truth.
- Workspace hierarchy tag storage remains under `src/app/models/file/hierarchy/tags`.
- General hub/package/storage validators remain under `src/app/models/file/validator`.
- Resource import orchestration remains under `src/app/models/editor/resource`; only RAW `<resource ... />` tag
  construction and insertion policy live here.

## 한국어

이 섹션은 위 README 내용을 한국어로 확인하기 위한 하단 요약이다.

- 대상: ``src/app/models/editor/tags`` (`docs/src/app/models/editor/tags/README.md`)
- 위치: `docs/src/app/models/editor/tags`
- 역할: 이 파일은 해당 디렉터리나 모듈의 구조, 책임, 운영 규칙, 검증 기준을 설명한다.
- 기준: 파일 경로, 명령, API 이름, 세부 변경 이력은 위 영어 본문을 원문 기준으로 유지한다.
- 변경 시: 위 영어 본문을 수정하면 이 한국어 하단 섹션도 함께 최신 상태로 맞춘다.
- 최근 기준: inline formatting 재적용은 기존 RAW inline-style 태그 토큰 경계를 먼저 정규화한 뒤 source
  mutation을 생성해 숨겨진 `<highlight>`류 태그가 쪼개져 노출되지 않도록 한다.
