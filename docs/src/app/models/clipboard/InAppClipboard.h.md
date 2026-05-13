# `src/app/models/clipboard/InAppClipboard.h`

## Responsibility

Declares the QObject exposed to QML as `inAppClipboard`.

## Contract

- Holds one clipboard resource snapshot.
- Captures supported system clipboard payloads through `captureSystemClipboardResource()`.
- Accepts app-internal non-image resources through `setResourceLocalFile(...)`, `setResourceBytes(...)`, and
  `setResourceText(...)`.
- Exposes URL/file import and clipboard-resource paste invokables:
  `importUrlsWithConflictPolicy(...)`, `importUrlsForEditor(...)`,
  `refreshClipboardResourceAvailabilitySnapshot()`, and `importClipboardResourceForEditor()`.
- Keeps current hub path, busy state, last error, conflict policy, import completion signal, and resource reload
  callback in C++.

## 한국어

- 이 객체가 삭제된 `ResourcesImportController`의 import 표면을 대체한다.
- QML context 이름은 `inAppClipboard`다.
- 앱 내부에서는 이미지뿐 아니라 local file, raw bytes, text payload도 단일 clipboard resource로 넣을 수 있다.
- `ContentViewLayout.qml`은 이 객체로 clipboard 리소스를 `.wsresource`로 먼저 등록한 뒤,
  `NoteEditorDocumentSession.insertImportedResourcesIntoSource(...)`에 반환 metadata만 전달한다.
