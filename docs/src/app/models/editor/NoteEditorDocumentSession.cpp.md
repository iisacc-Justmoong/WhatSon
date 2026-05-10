# `src/app/models/editor/NoteEditorDocumentSession.cpp`

## Responsibility

Implements the active note editor document session.

## Runtime Flow

1. Active-note changes are read from `NoteActiveStateTracker`.
2. `ContentsNoteManagementCoordinator` loads the selected note body as canonical editor-facing RAW source.
3. The source is written to a cache/session `.wsnsource` file.
4. QML binds that session file into LVRS `TextEditor.filePath` and passes the derived line count into the gutter.
5. When LVRS emits `syncFinished(path)`, QML calls `persistEditorFile(path)`, and the session delegates persistence
   back through `ContentsNoteManagementCoordinator` so `.wsnbody` is reserialized and `parsedLineCount` is refreshed.

## Guardrails

- The session creates blank/source session files outside the note package.
- The session keeps a file-path-to-note-context map so late sync signals from a previous editor file still persist to
  the note that originally owned that file.
- Re-selecting the same note keeps the existing session source file intact, so unsaved editor state is not overwritten
  by a redundant body reload.
- The session computes parsed RAW source line count in C++ so QML gutter code does not read or parse note files.
- Imported resource metadata is normalized through `WhatSonNoteBodyResourceTagGenerator` and inserted as standalone
  RAW resource lines by `insertImportedResourcesIntoSource(...)`.
- It must not expose the raw XML body file as the editor file path.

## 한국어

- 이 구현은 `.wsnbody` XML을 그대로 에디터에 띄우지 않는다.
- 선택된 노트의 본문을 RAW source로 파싱해 session file에 쓰고, LVRS 저장 이벤트를 다시 `.wsnbody`
  직렬화 경로로 연결한다.
- 거터 표시용 line count는 이 C++ 세션이 parsed RAW source에서 계산한다.
- clipboard/drop으로 들어온 resource metadata는 이 세션이 standalone `<resource ... />` source block으로
  변환한다.
