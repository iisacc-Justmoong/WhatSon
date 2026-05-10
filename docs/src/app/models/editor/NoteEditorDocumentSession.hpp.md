# `src/app/models/editor/NoteEditorDocumentSession.hpp`

## Responsibility

Declares the active note editor document session object.

## Contract

- Exposes `editorFilePath`, the parsed RAW source session file consumed by `LV.TextEditor.filePath`.
- Tracks the active note id and note directory path from `NoteActiveStateTracker`.
- Exposes `parsedLineCount` for view-only gutter numbering. The value is derived from the parsed RAW source text loaded
  or persisted by this C++ session.
- Exposes `loading`, `readOnly`, and `lastError` so QML can keep the native editor surface guarded while C++ loads or
  clears a note.
- Provides `persistEditorFile(path)` for the LVRS `syncFinished` hook.

## Guardrails

- The editor file path must not point at the raw `.wsnbody` XML document.
- `.wsnbody` parsing and serialization stay in C++ note/editor model code, not QML.

## 한국어

- 이 객체는 선택된 노트를 LVRS `TextEditor`에 연결하기 위한 C++ 세션 경계다.
- `editorFilePath`는 `.wsnbody` 원문이 아니라 parsed RAW source session file이어야 한다.
- `parsedLineCount`는 거터 표시용 라인 수이며 QML이 직접 파일을 읽거나 파싱하지 않게 한다.
- LVRS가 session file 저장을 끝내면 `persistEditorFile(...)`이 다시 `.wsnbody` 저장 경로로 넘긴다.
