# `src/app/models/editor/TagInsertionWriter.cpp`

## Responsibility

Implements persisted tag insertion for local `.wsnote` packages.

## Runtime Flow

1. Validate the note id and note directory path.
2. Read the current note through `WhatSonLocalNoteFileStore`.
3. Apply the requested static tag through `SetTag` against the note body source text.
4. Persist the mutated source through `WhatSonLocalNoteFileStore::updateNote(...)`.
5. Return and emit a result map containing the mutated source, actual `.wsnbody` text, cursor position, and error state.

## Guardrails

- Unsupported tags are rejected before the note body is written.
- `resource` remains a standalone body node because serialization still flows through `WhatSonNoteBodyPersistence`.
- Header metadata, modified count, and body-derived statistics are updated by the note file store, not by ad-hoc XML
  string replacement.

## 한국어

- 이 구현은 `SetTag`의 source 변환 결과를 실제 `.wsnbody` 저장 경로에 연결한다.
- XML 문자열을 직접 조작하지 않고 기존 노트 파일 store와 body persistence를 사용한다.
