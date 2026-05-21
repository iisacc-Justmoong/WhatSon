# `src/app/models/clipboard/ClipboardEditorPaste.h`

## Responsibility

Declares the QObject used by the note editor input filter to paste clipboard resources into the editor.

## Contract

- Exposes `pasteImageResourceIntoEditor(...)` as the narrow QML invokable for image resource paste.
- Does not own editor item event-filter lifetime. `EditorInputCommandFilter` invokes this object when a paste shortcut
  needs resource paste orchestration.
- Accepts the existing `InAppClipboardManager` and `NoteEditorDocumentSession` context objects instead of owning them.
- Requires that the supplied note session already has an active note and mounted editor file; image paste is a persistence
  boundary, not a temporary rich-text insertion.
- Returns a result map with `valid`, `nativePaste`, `editorDocumentText`, `bodySourceText`, `cursorPosition`,
  `importedEntries`, `reloadSucceeded`, and `errorMessage` fields.

## 한국어

- 현재 구현 범위는 이미지 clipboard resource를 `.wsresource`로 만든 뒤 노트 editor source에 `<resource ... />`
  참조를 삽입하는 것이다.
- active note session이 없으면 임시 프레임을 만들지 않고 실패를 반환한다.
- editor item event filter는 `EditorInputCommandFilter`가 소유한다. 이 객체는 paste shortcut이 실제 이미지
  resource paste로 판정된 뒤 호출되는 orchestration 객체다.
- 비이미지 clipboard resource는 아직 editor resource paste로 처리하지 않고 event를 consume하지 않아 native paste
  경로에 남긴다.
