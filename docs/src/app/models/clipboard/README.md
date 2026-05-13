# `src/app/models/clipboard`

## Role

This shard owns the in-app clipboard resource state and the resource import bridge used by note-editor paste and
macOS file import.

## Components

- `ClipboardResourceImport.*`
  Maps file names and MIME types, including non-image documents, links, audio/music, video, 3D models, and archives,
  to the same resource taxonomy used by `.wsresources` packages.
- `InAppClipboard.*`
  Stores one in-app clipboard resource at a time, captures supported system clipboard payloads, accepts app-internal
  local files/bytes/text, and exposes the QML context object named `inAppClipboard`.
- `ClipboardResourcePackageImport.cpp`
  Persists local files or materialized clipboard payloads into `.wsresources/<id>.wsresource`, updates
  `Resources.wsresources`, handles duplicate import policy, and returns editor insertion metadata.

## Guardrails

- Do not reintroduce `ResourcesImportController`; import orchestration is part of `InAppClipboard`.
- QML may call the narrow invokables, but MIME inspection, package creation, conflict handling, and reload callbacks
  stay in C++.
- Editor paste must first persist a `.wsresource` package, then pass only returned metadata to
  `NoteEditorDocumentSession.insertImportedResourcesIntoSource(...)`.

## Tests

- `test/cpp/suites/in_app_clipboard_tests.cpp`
- `test/cpp/suites/in_app_clipboard_resource_import_tests.cpp`

## 한국어

- 이 shard는 앱 내부 clipboard 리소스 상태와 resource import bridge를 소유한다.
- 한 번에 여러 clipboard 항목을 저장하지 않고, 현재 붙여넣을 수 있는 단일 리소스의 file type과 resource taxonomy
  mapping 및 payload만 유지한다.
- 이미지만이 아니라 문서, 링크 HTML/text, 오디오/음악, 비디오, 3D 모델, 압축 파일 같은 지원 resource taxonomy
  항목도 앱 내부 clipboard로 받을 수 있다.
- QML은 `inAppClipboard` context object의 좁은 invokable만 호출하며, 실제 MIME 판별, 패키지 생성, 충돌 처리,
  `.wsresources` 갱신은 C++에서 수행한다.
