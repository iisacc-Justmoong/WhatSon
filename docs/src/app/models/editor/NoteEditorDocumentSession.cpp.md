# `src/app/models/editor/NoteEditorDocumentSession.cpp`

## Responsibility

Implements the active note editor document session.

## Runtime Flow

1. Active-note changes are read from `NoteActiveStateTracker`.
2. `ContentsNoteManagementCoordinator` loads the selected note body as canonical editor-facing RAW source.
3. The source is projected into editor HTML and written to a cache/session `.wsnsource` file so LVRS
   `TextEditor` receives explicit rich-text line breaks.
4. QML binds that session file into LVRS `TextEditor.filePath`, keeps the parsed source line count as session metadata,
   and uses that value as the gutter delegate count. The sibling editor supplies only rendered start positions for
   those parsed source lines.
5. When LVRS emits `syncFinished(path)`, QML calls `persistEditorFile(path)`, the session converts the editor document
   HTML back into canonical source text, and then delegates persistence through `ContentsNoteManagementCoordinator`
   so `.wsnbody` is reserialized and `parsedLineCount` is refreshed.
6. Editor format shortcuts call `insertFormatTagIntoSource(...)`; the session converts the current editor document back
   to source, maps the rendered selection to RAW visible-character positions, applies `SetTag`, returns a fresh editor
   HTML projection, and maps the source cursor back to the rendered editor cursor position.

## Guardrails

- The session creates blank/editor session files outside the note package.
- The session keeps a file-path-to-note-context map so late sync signals from a previous editor file still persist to
  the note that originally owned that file.
- Re-selecting the same note keeps the existing session file intact, so unsaved editor state is not overwritten
  by a redundant body reload.
- The session computes parsed RAW source line count in C++ so QML gutter code does not read or parse note files. The
  gutter uses that metadata as its row count and must not derive row count from LVRS rendered wrap-line geometry.
- Imported resource metadata is normalized through `WhatSonNoteBodyResourceTagGenerator` and inserted as standalone
  RAW resource lines by `insertImportedResourcesIntoSource(...)`; the result also includes an editor HTML projection
  so QML can refresh the LVRS rich-text surface without reintroducing raw newline rendering.
- Static format tags are inserted by `SetTag` through `insertFormatTagIntoSource(...)`. QML supplies only the tag name,
  current editor document text, cursor, and selection length; source mutation, same-tag toggle removal, projection, and
  line-count refresh stay here.
- It must not expose the raw XML body file as the editor file path.

## 한국어

- 이 구현은 `.wsnbody` XML을 그대로 에디터에 띄우지 않는다.
- 선택된 노트의 본문을 RAW source로 파싱한 뒤 LVRS `TextEditor`가 줄바꿈을 보존해 렌더할 수 있는 editor HTML
  session file로 투영하고, LVRS 저장 이벤트는 다시 canonical source로 복원해 `.wsnbody` 직렬화 경로로 연결한다.
- parsed RAW source line count는 이 C++ 세션이 계산한다. 거터 표시 row 개수는 이 metadata만 따른다.
  paragraph wrap으로 생긴 rendered row count는 거터 row count에 참여하지 않는다.
- clipboard/drop으로 들어온 resource metadata는 이 세션이 standalone `<resource ... />` source block으로
  변환한다.
- 포맷 단축키는 이 세션의 `insertFormatTagIntoSource(...)`로 들어오며, 보이는 selection 좌표를 RAW source
  좌표로 변환한 뒤 `SetTag` 결과를 editor HTML로 다시 투영한다. 같은 태그가 정확히 감싼 selection이면
  `SetTag`가 wrapper를 제거하는 toggle 결과를 반환한다.
