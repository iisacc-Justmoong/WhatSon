# `src/app/models/editor/EditorInputCommandFilter.cpp`

## Responsibility

Implements the editor item key event filter for command-style editor input.

## Notes

- Installs itself on the public LVRS editor item supplied by `TextEditor.qml`.
  It consumes native key events before they reach `TextEdit` only when the active RAW source cursor is on a handled
  semantic boundary operation.
- Delegates callout source behavior to `NoteEditorDocumentSession.handleCalloutBoundaryKeyInSource(...)`.
- Delegates image resource paste orchestration to `ClipboardEditorPaste.pasteImageResourceIntoEditor(...)`.
- For image resource paste, reads `editorCursorPosition` from the QML editor wrapper and sends a collapsed selection to
  `ClipboardEditorPaste`, so paste inserts at the caret instead of replacing stale selected content.
- For collapsed boundary commands, reads `editorCursorPosition` from the QML editor wrapper instead of reusing a stale
  `selectionStart`.
- Uses `replaceEditorDocumentText(nextText, nextCursorPosition)` on the QML editor wrapper to apply C++-projected
  editor HTML and restore the cursor.

## 한국어

- 이 구현은 editor item에 설치되는 하나의 C++ event filter다.
- `ClipboardEditorPaste` 이름 아래 숨어 있던 editor-wide key interception 책임을 분리한 객체다.
- callout boundary는 `NoteEditorDocumentSession`으로 위임하고, 이미지 resource paste는 `ClipboardEditorPaste`로
  위임한다.
- 이미지 resource paste는 QML wrapper의 `editorCursorPosition`을 기준으로 하고 selection length를 0으로 전달한다.
  따라서 stale selection이 남아 있어도 붙여넣기가 주변 텍스트나 frame을 치환하지 않는다.
- selection이 접힌 boundary 명령은 `selectionStart`가 아니라 QML wrapper의 `editorCursorPosition`을 넘겨 실제 caret
  위치에 RAW source mutation이 적용되도록 한다.
