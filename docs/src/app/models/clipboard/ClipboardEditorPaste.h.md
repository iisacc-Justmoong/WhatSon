# `src/app/models/clipboard/ClipboardEditorPaste.h`

## Responsibility

Declares the QObject used by the note editor's consumable paste owner to paste clipboard resources into the editor.

## Contract

- Exposes `pasteImageResourceIntoEditor(...)` as the narrow QML invokable for image resource paste.
- Exposes `attachEditorPasteOwner(...)` so the QML editor wrapper can install this QObject as an event filter on the
  public LVRS editor item.
- Consumes handled image paste key events in C++ so a successful resource paste is not followed by native `TextEdit`
  paste for the same physical `Cmd+V` gesture.
- Also consumes plain Backspace/Enter key events only when `NoteEditorDocumentSession` reports that the key was handled
  as a callout boundary operation. Other Backspace/Enter positions remain native editor input.
- Accepts the existing `InAppClipboardManager` and `NoteEditorDocumentSession` context objects instead of owning them.
- Returns a result map with `valid`, `nativePaste`, `editorDocumentText`, `bodySourceText`, `cursorPosition`,
  `importedEntries`, `reloadSucceeded`, and `errorMessage` fields.

## 한국어

- `TextEditor.qml` wrapper가 공개 LVRS editor item에 event filter로 붙이는 editor paste owner다.
- 현재 구현 범위는 이미지 clipboard resource를 `.wsresource`로 만든 뒤 노트 editor source에 `<resource ... />`
  참조를 삽입하는 것이다.
- 이미지 resource paste를 처리하면 실제 key event를 consume해 native paste가 중복으로 이어지지 않게 한다.
- 비이미지 clipboard resource는 아직 editor resource paste로 처리하지 않고 event를 consume하지 않아 native paste
  경로에 남긴다.
- 콜아웃 시작점 Backspace와 콜아웃 내부 Enter/Return은 `NoteEditorDocumentSession`이 처리했다고 응답한 경우에만
  consume한다. 일반 텍스트 위치의 Backspace/Enter는 native editor 입력으로 남긴다.
