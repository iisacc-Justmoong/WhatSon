# `src/app/qml/view/panels/ContentViewLayout.qml`

## Role
`ContentViewLayout.qml` is the content slot surface used by the restored desktop/mobile shell. It exists only to place
the three contents views side by side.

## Composition
- `ContentsView.Gutter`
- `ContentsView.TextEditor`
- `ContentsView.Minimap`

The text editor view is rooted in LVRS `TextEditor` and keeps `filePath` empty.

## Shell Inputs
The restored shell can still assign legacy layout inputs such as note-list, sidebar, resource-import, and calendar
handles into this component. They are compatibility inputs only; `ContentViewLayout.qml` does not use them to mount a
TextEditor backend, parser, projection, renderer, persistence path, resource editor, or calendar page.

`editorViewModeController` remains removed and must not be reintroduced.

## Guardrails
- Do not add parser, projection, rendering, persistence, snapshot, resource, calendar, or editor view-mode wiring here.
- Do not mount `LV.CodeEditor`, raw `TextEdit`, RichText overlays, or legacy editor backend objects.
- Keep the component as a view-only LVRS composition layer.

## Tests
- `test/cpp/suites/qml_contents_view_tests.cpp`
- `test/cpp/suites/qml_inline_format_editor_tests.cpp`

## 한국어

- 이 파일은 restored workspace shell의 content slot surface다.
- 내부 배치는 거터, `LV.TextEditor` wrapper, 미니맵으로 끝난다.
- shell 호환 입력은 받을 수 있지만 노트 세션, 저장, 프로젝션, 렌더링, 캘린더, editor view mode 백엔드를 mount하지 않는다.
