# `src/app/models/clipboard/ClipboardEditorPaste.cpp`

## Responsibility

Implements image resource paste orchestration for the note editor.

## Notes

- Refreshes the in-app clipboard resource snapshot, currently accepts only `image` resources, and requests native paste
  fallback for unsupported resource types.
- Requires an active `NoteEditorDocumentSession` with a mounted editor file before it imports clipboard data. Without that
  persistence context it returns a handled `note-session` failure so a temporary rich-text image frame is never inserted.
- Does not install a C++ event filter. `EditorInputCommandFilter` owns key interception and calls
  `pasteImageResourceIntoEditor(...)` only for paste shortcut candidates.
- Returns a `stage` field for every handled result so QML can distinguish capture failure, unsupported resources,
  import failure, source insertion failure, reload failure, and completed paste without reaching into lower-level
  objects.
- Imports the clipboard image through `InAppClipboardManager`, which creates the `.wsresources/<id>.wsresource`
  package and updates `Resources.wsresources`.
- Delegates editor/body source mutation to `NoteEditorDocumentSession.insertImportedResourcesIntoSource(...)`, which uses
  the current editor snapshot and queues the corresponding `.wsnbody` write instead of creating a temporary rich-text
  frame.
- Image resource paste is insertion, not replacement. `EditorInputCommandFilter` passes the caret and a collapsed
  selection, and `NoteEditorDocumentSession` does not use `selectionLength` as a deletion range for surrounding source.
- Reloads imported resources after a valid insertion plan is produced, while preserving the editor insertion result if
  reload reports a non-fatal failure.

## 한국어

- 에디터 붙여넣기 시점에 clipboard 이미지가 앱 내부 `.wsresource`로 들어가고, 본문에는 metadata 기반
  `<resource ... />` 태그만 삽입되도록 조율한다. 실제 source 삽입과 `.wsnbody` 쓰기는
  `NoteEditorDocumentSession`이 현재 editor snapshot 기준으로 처리한다.
- active note session과 mounted editor file이 없으면 clipboard import를 시작하지 않는다. 이 경우 native image
  paste로 빠지지 않고 handled failure를 반환해 RAW에 없는 임시 이미지 프레임이 생기지 않게 한다.
- 반환 map의 `stage`는 `capture`, `unsupported-resource`, `import`, `source-insertion`, `reload`, `completed`
  같은 실패/완료 단계를 담아 QML이 native paste fallback과 실제 처리 실패를 구분할 수 있게 한다.
- QML은 반환된 `editorDocumentText`와 `cursorPosition`만 LVRS `TextEditor`에 반영한다.
- 이미지 resource paste는 replacement가 아니라 insertion이다. `EditorInputCommandFilter`는 caret 위치와 collapsed
  selection을 넘기고, `NoteEditorDocumentSession`은 selection length를 주변 source 삭제 범위로 쓰지 않는다.
- key event consume 여부는 `EditorInputCommandFilter`가 결정한다. 이 객체는 붙여넣기 orchestration 결과에
  `nativePaste: true`가 있으면 native LVRS paste 경로를 남긴다.
