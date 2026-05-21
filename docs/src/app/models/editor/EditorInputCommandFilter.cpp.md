# `src/app/models/editor/EditorInputCommandFilter.cpp`

## Responsibility

Implements the editor item key event filter for command-style editor input.

## Notes

- Installs itself on the public LVRS editor item supplied by `TextEditor.qml`.
  It consumes native key events before they reach `TextEdit` only when the active RAW source cursor is on a handled
  semantic boundary operation.
- Delegates callout source behavior to `NoteEditorDocumentSession.handleCalloutBoundaryKeyInSource(...)`.
- Delegates explicit empty paragraph Backspace to `NoteEditorDocumentSession.handleEmptyParagraphBoundaryKeyInSource(...)`
  so hidden placeholders do not require a second key press before the gutter line count changes.
- Delegates image resource paste orchestration to `ClipboardEditorPaste.pasteImageResourceIntoEditor(...)`.
- For collapsed selections, reads `editorCursorPosition` from the QML editor wrapper and sends that caret position to
  paste and boundary commands instead of reusing a stale `selectionStart`.
- Uses `replaceEditorDocumentText(nextText, nextCursorPosition)` on the QML editor wrapper to apply C++-projected
  editor HTML and restore the cursor.

## 한국어

- 이 구현은 editor item에 설치되는 하나의 C++ event filter다.
- `ClipboardEditorPaste` 이름 아래 숨어 있던 editor-wide key interception 책임을 분리한 객체다.
- callout boundary와 명시적 빈 paragraph Backspace는 `NoteEditorDocumentSession`으로 위임하고, 이미지 resource
  paste는 `ClipboardEditorPaste`로 위임한다.
- selection이 접힌 상태에서는 `selectionStart`가 아니라 QML wrapper의 `editorCursorPosition`을 넘겨 실제 caret
  위치에 RAW source mutation이 적용되도록 한다.
