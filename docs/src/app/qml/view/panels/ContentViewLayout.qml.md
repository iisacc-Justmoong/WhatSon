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

The gutter receives the same selected source context plus `NoteEditorDocumentSession.parsedLineCount`, the sibling
editor's rendered line count, the editor viewport offset, the editor metric revision, the current cursor line index,
and a per-line metric provider from the sibling `TextEditor.qml`. It does not read or parse the note file itself, and
its delegate count follows the rendered editor surface rather than the parsed RAW source line count.

The minimap receives the sibling editor's rich-text document, viewport geometry, font tokens, and a view-local scroll
hook so it can render a VSCode-style right-side miniature and viewport thumb without owning parser or persistence
state.

Clipboard image paste is handled as a narrow command flow: refresh clipboard image availability, call
`ResourcesImportController.importClipboardImageForEditor()`, ask `NoteEditorDocumentSession` to compute the canonical
RAW resource insertion and editor HTML projection, replace the LVRS document with that C++ result, then run the
deferred resources reload.

Editor formatting is handled through the same narrow command shape. Focus-gated shortcuts for `bold`, `italic`,
`underline`, `strikethrough`, `highlight`, and `break` call
`NoteEditorDocumentSession.insertFormatTagIntoSource(...)` with the sibling editor's document, cursor, and selection
metadata, then replace the LVRS document with the C++-projected editor HTML result. Highlight is bound to
`Meta+Shift+E` / `Ctrl+Shift+E`, and break is bound to `Meta+Shift+B` / `Ctrl+Shift+B`.

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
- Keep gutter wiring limited to selected-note metadata, parsed source line count, rendered editor line count, editor
  metric provider/revision, fallback editor visual line height, current cursor line index, and viewport offset.
- Keep minimap wiring limited to editor document text, viewport geometry, source font tokens, and the editor viewport
  scroll hook.
- Keep paste handling limited to command dispatch; resource import, tag construction, and persistence policy stay in
  C++ objects.
- Keep format shortcut handling limited to command dispatch; static tag allow-lists, source mutation, editor HTML
  projection, and persistence policy stay in `SetTag`, `NoteEditorDocumentSession`, and note-body persistence.
- Do not mount `LV.CodeEditor`, raw `TextEdit`, RichText overlays, or legacy editor backend objects.
- Keep the component as a view-only LVRS composition layer.

## Tests
- `test/cpp/suites/qml_contents_view_tests.cpp`
- `test/cpp/suites/qml_inline_format_editor_tests.cpp`

## 한국어

- 이 파일은 restored workspace shell의 content slot surface다.
- 내부 배치는 거터, `LV.TextEditor` wrapper, 미니맵으로 끝난다.
- shell 호환 입력은 받을 수 있고 active note의 editor HTML session file만 `LV.TextEditor.filePath`로 넘긴다.
- 거터에는 선택 노트 경로, parsed source line count, rendered editor line count, editor line metric provider/revision,
  fallback editor visual line height, current cursor line index, viewport offset만 전달한다.
- 미니맵에는 editor document text, viewport geometry, source font token, editor viewport scroll hook만 전달한다.
- 이미지 paste는 `ResourcesImportController`와 `NoteEditorDocumentSession`의 C++ source/editor HTML 결과를 이어
  붙이는 얇은 command wiring으로 제한한다.
- 포맷 단축키는 `NoteEditorDocumentSession.insertFormatTagIntoSource(...)`의 C++ source/editor HTML 결과를 이어
  붙이는 얇은 command wiring으로 제한한다. 하이라이트는 `Cmd+Shift+E`, 브레이크는 `Cmd+Shift+B`를 기준으로
  하고, 비 macOS 변형으로 같은 `Ctrl+Shift+E/B`도 받는다.
- `.wsnbody` parse/serialize는 C++ `NoteEditorDocumentSession`에 맡기며 프로젝션, 렌더링, 캘린더,
  editor view mode 백엔드는 mount하지 않는다.
