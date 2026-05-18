# `src/app/models/editor/EditorInputCommandFilter.cpp`

## Responsibility

Implements the editor item key event filter for command-style editor input.

## Notes

- Installs itself on the public LVRS editor item supplied by `TextEditor.qml`.
- Dispatch order is agenda boundary first, callout boundary second, paste third. This keeps plain Backspace/Enter from
  reaching native `TextEdit` only when the active RAW source cursor is on a handled semantic boundary operation.
- Delegates agenda task source behavior to `NoteEditorDocumentSession.handleAgendaBoundaryKeyInSource(...)`.
- Delegates callout source behavior to `NoteEditorDocumentSession.handleCalloutBoundaryKeyInSource(...)`.
- Delegates image resource paste orchestration to `ClipboardEditorPaste.pasteImageResourceIntoEditor(...)`.
- Uses `replaceEditorDocumentText(nextText, nextCursorPosition)` on the QML editor wrapper to apply C++-projected
  editor HTML and restore the cursor.

## 한국어

- 이 구현은 editor item에 설치되는 하나의 C++ event filter다.
- `ClipboardEditorPaste` 이름 아래 숨어 있던 editor-wide key interception 책임을 분리한 객체다.
- `Cmd/Ctrl+V` 입력은 paste 객체로 위임하고, agenda task 및 callout `Backspace`/`Enter` 입력은 note editor
  session으로 위임한다.
