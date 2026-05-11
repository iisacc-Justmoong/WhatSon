# `src/app/qml/view/panels/ContentViewLayout.qml`

## Role
`ContentViewLayout.qml` is the content slot surface used by the restored desktop/mobile shell. It exists only to place
the three contents views side by side.

## Composition
- `ContentsView.Gutter`
- `ContentsView.TextEditor`
- `ContentsView.Minimap`

The text editor view is rooted in LVRS `TextEditor` and receives an editor HTML session file path from
`NoteEditorDocumentSession.editorFilePath`.

The gutter receives the same selected source context plus `NoteEditorDocumentSession.parsedLineCount`, the measured
editor visual line height, and the editor viewport offset. It does not read or parse the note file itself.

Clipboard image paste is handled as a narrow command flow: refresh clipboard image availability, call
`ResourcesImportController.importClipboardImageForEditor()`, ask `NoteEditorDocumentSession` to compute the canonical
RAW resource insertion and editor HTML projection, replace the LVRS document with that C++ result, then run the
deferred resources reload.

## Shell Inputs
The restored shell can still assign legacy layout inputs such as note-list, sidebar, resource-import, and calendar
handles into this component. `noteEditorSession` is consumed only to bind the editor session file into `LV.TextEditor`
and to notify the C++ session when LVRS finishes syncing that session file. Other inputs remain
compatibility handles and are not used to mount parser, projection, renderer, resource editor, or calendar page logic.

`editorViewModeController` remains removed from this component and must not be reintroduced as a TextEditor backend.

## Guardrails
- Do not add parser, projection, rendering, snapshot, resource editor, calendar, or editor view-mode wiring here.
- Keep file access limited to the `NoteEditorDocumentSession.editorFilePath -> LV.TextEditor.filePath` binding and
  `syncFinished -> NoteEditorDocumentSession.persistEditorFile(...)` hook.
- Keep gutter wiring limited to selected-note metadata, parsed line count, measured editor visual line height, and
  viewport offset.
- Keep paste handling limited to command dispatch; resource import, tag construction, and persistence policy stay in
  C++ objects.
- Do not mount `LV.CodeEditor`, raw `TextEdit`, RichText overlays, or legacy editor backend objects.
- Keep the component as a view-only LVRS composition layer.

## Tests
- `test/cpp/suites/qml_contents_view_tests.cpp`
- `test/cpp/suites/qml_inline_format_editor_tests.cpp`

## 한국어

- 이 파일은 restored workspace shell의 content slot surface다.
- 내부 배치는 거터, `LV.TextEditor` wrapper, 미니맵으로 끝난다.
- shell 호환 입력은 받을 수 있고 active note의 editor HTML session file만 `LV.TextEditor.filePath`로 넘긴다.
- 거터에는 선택 노트 경로, parsed line count, 실제 editor visual line height, viewport offset만 전달한다.
- 이미지 paste는 `ResourcesImportController`와 `NoteEditorDocumentSession`의 C++ source/editor HTML 결과를 이어
  붙이는 얇은 command wiring으로 제한한다.
- `.wsnbody` parse/serialize는 C++ `NoteEditorDocumentSession`에 맡기며 프로젝션, 렌더링, 캘린더,
  editor view mode 백엔드는 mount하지 않는다.
