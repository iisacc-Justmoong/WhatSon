# `src/app/models/clipboard/ClipboardEditorPaste.cpp`

## Responsibility

Implements image resource paste orchestration for the note editor.

## Notes

- Refreshes the in-app clipboard resource snapshot, currently accepts only `image` resources, and requests native paste
  fallback for unsupported resource types.
- Returns a `stage` field for every handled result so QML can distinguish capture failure, unsupported resources,
  import failure, source insertion failure, reload failure, and completed paste without reaching into lower-level
  objects.
- Imports the clipboard image through `InAppClipboardManager`, which creates the `.wsresources/<id>.wsresource`
  package and updates `Resources.wsresources`.
- Delegates editor/body source mutation to `NoteEditorDocumentSession.insertImportedResourcesIntoSource(...)`.
- Reloads imported resources after a valid insertion plan is produced, while preserving the editor insertion result if
  reload reports a non-fatal failure.

## 한국어

- 에디터 붙여넣기 시점에 clipboard 이미지가 앱 내부 `.wsresource`로 들어가고, 본문에는 metadata 기반
  `<resource ... />` 태그만 삽입되도록 조율한다.
- 반환 map의 `stage`는 `capture`, `unsupported-resource`, `import`, `source-insertion`, `reload`, `completed`
  같은 실패/완료 단계를 담아 QML이 native paste fallback과 실제 처리 실패를 구분할 수 있게 한다.
- QML은 반환된 `editorDocumentText`와 `cursorPosition`만 LVRS `TextEditor`에 반영한다.
