# `src/app/qml/view/panels/ContentViewLayout.qml`

## Role
`ContentViewLayout.qml` is the content slot surface used by the restored desktop/mobile shell. It exists only to place
the three contents views side by side.

## Composition
- `ContentsView.Gutter`
- `ContentsView.TextEditor`
- `ContentsView.Minimap`

The text editor view is rooted in LVRS `TextEditor` and receives a parsed RAW source session file path from
`NoteEditorDocumentSession.editorFilePath`.

## Shell Inputs
The restored shell can still assign legacy layout inputs such as note-list, sidebar, resource-import, and calendar
handles into this component. `noteEditorSession` is consumed only to bind the parsed source session file into
`LV.TextEditor` and to notify the C++ session when LVRS finishes syncing that session file. Other inputs remain
compatibility handles and are not used to mount parser, projection, renderer, resource editor, or calendar page logic.

`editorViewModeController` remains removed and must not be reintroduced.

## Guardrails
- Do not add parser, projection, rendering, snapshot, resource, calendar, or editor view-mode wiring here.
- Keep file access limited to the `NoteEditorDocumentSession.editorFilePath -> LV.TextEditor.filePath` binding and
  `syncFinished -> NoteEditorDocumentSession.persistEditorFile(...)` hook.
- Do not mount `LV.CodeEditor`, raw `TextEdit`, RichText overlays, or legacy editor backend objects.
- Keep the component as a view-only LVRS composition layer.

## Tests
- `test/cpp/suites/qml_contents_view_tests.cpp`
- `test/cpp/suites/qml_inline_format_editor_tests.cpp`

## 한국어

- 이 파일은 restored workspace shell의 content slot surface다.
- 내부 배치는 거터, `LV.TextEditor` wrapper, 미니맵으로 끝난다.
- shell 호환 입력은 받을 수 있고 active note의 parsed RAW source session file만 `LV.TextEditor.filePath`로
  넘긴다.
- `.wsnbody` parse/serialize는 C++ `NoteEditorDocumentSession`에 맡기며 프로젝션, 렌더링, 캘린더,
  editor view mode 백엔드는 mount하지 않는다.
