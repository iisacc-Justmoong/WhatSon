# `src/app/models/clipboard`

## Role

This shard owns the in-app clipboard resource state and the resource import bridge used by note-editor paste and
macOS file import.

## Components

- `ClipboardResourceImport.*`
  Builds one clipboard resource import descriptor from a captured file, image, or payload after `FiletypeCapture`
  has normalized its file format.
- `FiletypeCapture.*`
  Maps file names and MIME types, including non-image documents, links, audio, video, 3D models, and archives,
  to the same resource taxonomy used by `.wsresources` packages.
  Music file extensions are normalized into the canonical `audio` / `Audio` taxonomy instead of a separate `music`
  resource type. Platform image clipboard MIME names such as `application/x-qt-image` and `com.apple.tiff` are also
  detected here before `InAppClipboardManager` materializes the resource.
- `InAppClipboardManager.*`
  Captures supported system clipboard payloads, accepts app-internal local files/bytes/text, persists local files or
  materialized clipboard payloads into
  `.wsresources/<id>.wsresource`, updates `Resources.wsresources`, handles duplicate import policy, returns editor
  insertion metadata, and exposes the QML context object named `inAppClipboard`. Default materialized clipboard payloads
  are renamed to fresh 32-character alphanumeric package and asset names before duplicate preflight and persistence.
- `ClipboardEditorPaste.*`
  Owns note-editor paste orchestration for clipboard resources. The first implemented path is image paste: capture the
  current resource snapshot, import it as a `.wsresource`, ask `NoteEditorDocumentSession` for the editor/body source
  insertion, and report whether QML should fall back to native paste with a stage string for diagnostics. It does not
  own editor item event-filter lifetime; `EditorInputCommandFilter` calls it for handled paste shortcut candidates.
- `InAppClipboardStore.*`
  Stores one in-app clipboard resource snapshot at a time and emits `resourceChanged()` when the manager updates or
  clears that snapshot.

## Guardrails

- Do not reintroduce `ResourcesImportController`; import orchestration is part of `InAppClipboardManager`.
- QML may call the narrow paste/apply invokables, but MIME inspection, package creation, conflict handling, source
  insertion planning, and reload callbacks stay in C++.
- Clipboard availability refresh must read the current system clipboard even when a previous in-app snapshot exists, so
  a newly captured screenshot replaces stale resource state before editor paste runs. When current capture fails, the
  stale snapshot is cleared instead of being used for editor paste.
- Platform image MIME payloads and image data URLs must be extracted before generic text/html or byte-resource capture.
- Repeated screenshot paste must not reuse `clipboard-resource.*` as the persisted package or asset name; the manager
  assigns a fresh 32-character alphanumeric id and stores the asset under the same stem before conflict preflight can
  reject the default temporary file name.
- File format detection belongs to `FiletypeCapture`, not `ClipboardResourceImport`.
- `InAppClipboardManager` must not own a raw `ClipboardResourceImport` member; single-resource state belongs to
  `InAppClipboardStore`.
- Do not split package import into a separate `ClipboardResourcePackageImport` object or source shard; the package
  pipeline is part of `InAppClipboardManager`.
- Editor paste must go through `ClipboardEditorPaste` so the `.wsresource` package is persisted before only returned
  metadata reaches `NoteEditorDocumentSession.insertImportedResourcesIntoSource(...)`.
- Image editor paste requires an active note session before import; the fallback path must not insert a temporary rich-text
  image frame that lacks a backing RAW `<resource ... />` line.
- Image editor paste must let `NoteEditorDocumentSession` insert the canonical `<resource ... />` tag into the current
  editor-derived `.wsnbody` source and queue that body write immediately; it must not rely on a later sync of a temporary
  frame.
- Callout boundary Backspace/Enter share the editor item event filter owned by `EditorInputCommandFilter`, but RAW
  source decisions must still live in `NoteEditorDocumentSession`.

## Tests

- `test/cpp/suites/in_app_clipboard_tests.cpp`
- `test/cpp/suites/in_app_clipboard_resource_import_tests.cpp`

## 한국어

- 이 shard는 앱 내부 clipboard 리소스 상태와 resource import bridge를 소유한다.
- 한 번에 여러 clipboard 항목을 저장하지 않고, 현재 붙여넣을 수 있는 단일 리소스의 file type과 resource taxonomy
  mapping 및 payload는 `InAppClipboardStore`에 유지한다.
- 이미지만이 아니라 문서, 링크 HTML/text, 오디오, 비디오, 3D 모델, 압축 파일 같은 지원 resource taxonomy
  항목도 `FiletypeCapture`를 통해 앱 내부 clipboard로 받을 수 있다.
- macOS/Qt 스크린샷 clipboard의 platform image MIME payload는 일반 text/html/bytes 리소스보다 먼저 이미지로
  추출되어야 한다.
- 음악 파일 확장자도 별도 `music` type이 아니라 canonical `audio`/`Audio` taxonomy로 들어간다.
- 기본 임시 이름인 `clipboard-resource.*`로 materialize된 clipboard payload는 persist 전에 32자 영문대소숫자
  package/asset 이름으로 바뀌며, duplicate preflight에서도 기본 임시 파일명 자체로 충돌하지 않는다.
- QML은 `ClipboardEditorPaste`의 좁은 editor paste invokable 결과만 에디터에 반영하며, 실제 MIME 판별,
  패키지 생성, 충돌 처리, source 삽입 계획, `.wsresources` 갱신은 C++ 객체들이 수행한다.
- 이미지 editor paste는 active note session이 있을 때만 import를 시작한다. RAW `<resource ... />` 줄에 연결되지
  않는 임시 rich-text 이미지 프레임은 허용하지 않는다.
- editor item event filter는 `EditorInputCommandFilter`가 소유한다. 콜아웃 경계 Backspace/Enter의
  `<callout>` RAW 판단과 source mutation은 `NoteEditorDocumentSession` 책임이다.
