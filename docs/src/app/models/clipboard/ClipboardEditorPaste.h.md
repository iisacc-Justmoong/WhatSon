# `src/app/models/clipboard/ClipboardEditorPaste.h`

## Responsibility

Declares the QObject used by note-editor QML paste shortcuts to paste clipboard resources into the editor.

## Contract

- Exposes `pasteImageResourceIntoEditor(...)` as the narrow QML invokable for image resource paste.
- Accepts the existing `InAppClipboardManager` and `NoteEditorDocumentSession` context objects instead of owning them.
- Returns a result map with `valid`, `nativePaste`, `editorDocumentText`, `bodySourceText`, `cursorPosition`,
  `importedEntries`, `reloadSucceeded`, and `errorMessage` fields.

## 한국어

- `ContentViewLayout.qml`의 `StandardKey.Paste` 경로가 호출하는 editor paste 객체다.
- 현재 구현 범위는 이미지 clipboard resource를 `.wsresource`로 만든 뒤 노트 editor source에 `<resource ... />`
  참조를 삽입하는 것이다.
- 비이미지 clipboard resource는 아직 editor resource paste로 처리하지 않고 native paste fallback을 요청한다.
