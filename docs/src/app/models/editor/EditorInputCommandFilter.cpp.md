# `src/app/models/editor/EditorInputCommandFilter.cpp`

## Responsibility

Implements the editor item key event filter for command-style editor input.

## Notes

- Installs itself on the public LVRS editor item supplied by `TextEditor.qml`.
  It consumes native key events before they reach `TextEdit` only when the active RAW source cursor is on a handled
  semantic boundary operation.
- Delegates callout source behavior to `NoteEditorDocumentSession.handleCalloutBoundaryKeyInSource(...)`.
- Delegates image resource paste orchestration to `ClipboardEditorPaste.pasteImageResourceIntoEditor(...)`.
- For image resource paste, forwards the current editor selection start and selection length to
  `ClipboardEditorPaste`, matching the 2026-05-19 paste command contract.
- For collapsed boundary commands, reads `editorCursorPosition` from the QML editor wrapper instead of reusing a stale
  `selectionStart`.
- Uses `replaceEditorDocumentText(nextText, nextCursorPosition)` on the QML editor wrapper to apply C++-projected
  editor HTML and restore the cursor.

## 한국어

- 이 구현은 editor item에 설치되는 하나의 C++ event filter다.
- `ClipboardEditorPaste` 이름 아래 숨어 있던 editor-wide key interception 책임을 분리한 객체다.
- callout boundary는 `NoteEditorDocumentSession`으로 위임하고, 이미지 resource paste는 `ClipboardEditorPaste`로
  위임한다.
- 이미지 resource paste는 2026-05-19 계약처럼 editor selection start와 selection length를 그대로 전달한다.
- selection이 접힌 boundary 명령은 `selectionStart`가 아니라 QML wrapper의 `editorCursorPosition`을 넘겨 실제 caret
  위치에 RAW source mutation이 적용되도록 한다.
- C++ 결과 HTML은 QML wrapper의 `replaceEditorDocumentText(...)`로 반영한다.
