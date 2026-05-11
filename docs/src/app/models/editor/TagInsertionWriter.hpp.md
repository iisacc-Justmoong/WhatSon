# `src/app/models/editor/TagInsertionWriter.hpp`

## Responsibility

Declares the persisted tag insertion writer object.

## API Contract

- `tagName` stores the configured static tag name, delegated to `SetTag`.
- `insertIntoNote(...)` writes the configured tag into a local note body.
- `insertNamedTagIntoNote(...)` writes an explicit static tag into a local note body.
- Results are returned as a `QVariantMap` and emitted through `tagWriteFinished(...)`.

## Boundaries

- The object receives note id, note directory path, cursor position, and selection length from callers.
- It does not inspect `LV.TextEditor` directly.
- It does not decide the current active note.

## 한국어

- 이 헤더는 실제 `.wsnbody` 태그 삽입 writer의 QML/Qt 호출 표면을 정의한다.
- 현재 선택 노트나 QML 이벤트는 호출자가 넘기며, 이 객체는 로컬 노트 파일 갱신에 집중한다.
