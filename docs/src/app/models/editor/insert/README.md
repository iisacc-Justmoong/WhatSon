# `src/app/models/editor/insert`

## Responsibility

Owns persisted editor insertion command objects.

## Current Contract

- `TagInsertionWriter` is the tag insertion writer for local note packages.
- It does not own tag template policy. Static tag mutation stays in `SetTag`.
- It does not own active-note tracking, editor cursor acquisition, or QML dispatch. Callers provide the target note
  context, cursor position, and selection length.
- It reads the selected local `.wsnote`, applies `SetTag` to the note body source text, and persists the result through
  `WhatSonLocalNoteFileStore`, which serializes the actual `.wsnbody` file.

## 한국어

- 이 디렉터리는 편집기 삽입 command 객체를 둔다.
- `TagInsertionWriter`는 실제 `.wsnbody` 쓰기만 담당하고, 태그 템플릿 계산은 `SetTag`에 맡긴다.
- 현재 노트 추적과 QML 버튼 dispatch는 이 객체의 책임이 아니다.
