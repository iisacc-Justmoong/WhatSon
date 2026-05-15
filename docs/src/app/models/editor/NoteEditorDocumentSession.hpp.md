# `src/app/models/editor/NoteEditorDocumentSession.hpp`

## Responsibility

Declares the active note editor document session object.

## Contract

- Exposes `editorFilePath`, the editor HTML session file consumed by `LV.TextEditor.filePath`.
- Tracks the active note id and note directory path from `NoteActiveStateTracker`.
- After a successful note mount, binds the selected note through `ContentsNoteManagementCoordinator` so open-count
  metadata is updated by the model/session queue rather than QML.
- Exposes `parsedLineCount` as canonical RAW source line metadata. The value is derived before source is projected into
  editor HTML or after synced editor HTML is converted back to source; QML uses this value as the gutter delegate
  count instead of LVRS rendered wrap-line count.
- Exposes `editorViewportWidth`, which QML binds from the public LVRS editor item width so image resource frames can
  render editor-width media inside the resource-frame container.
- Exposes `loading`, `readOnly`, and `lastError` so QML can keep the native editor surface guarded while C++ loads or
  clears a note.
- Routes note-entry/open RAW pulls through `file/sync/WhatSonEditorRawPullController`.
- Carries the loaded note `lastModified` timestamp with the editor-file context so RAW pushes can resolve
  multi-device conflicts by timestamp.
- Provides `recordEditorUserActivity()` and `requestActiveNoteIdleRawPull()` for the active-note idle pull loop. The
  idle loop pulls every 5000 ms while the editor surface is quiet, and the session only applies a filesystem body when
  its `lastModified` timestamp is newer than the loaded session timestamp.
- Emits `editorDocumentTextPulled(...)` when a newer filesystem body replaces the live editor document, and
  `editorFilesystemPullIgnored(...)` when an idle pull is stale, inactive, or failed.
- Provides `persistEditorFile(path)` for fallback file-based surface persistence.
- Provides `requestEditorIdleRawPush(...)` and `requestEditorModifiedCountRawPush(...)`, which forward editor-surface
  push triggers into `file/sync/WhatSonEditorRawPushController`.
- Forwards `hubFilesystemMutated()` from the note-management coordinator when a successful editor persistence writes
  a timestamped version diff to `.wsnversion`.
- Provides `insertImportedResourcesIntoSource(...)`, which receives resource package metadata already persisted by
  `InAppClipboardManager`, inserts canonical RAW `<resource ... />` calls at the current editor cursor/selection, and returns
  an editor HTML projection for the live LVRS surface.
- Provides `reprojectResourceFramesForEditorWidth(...)`, which recovers the current editor document as canonical source
  and re-renders only resource frames for a changed editor viewport width while preserving each frame's initial
  `data-frame-display-height`.
- Provides `insertFormatTagIntoSource(...)`, which applies a static editor format tag such as `bold`, `italic`,
  `underline`, `strikethrough`, `highlight`, or `break` through `SetTag`, then returns both canonical RAW source and
  an editor HTML projection for the live LVRS surface. The session keeps the loaded `.wsnbody` RAW source as the
  format mutation basis and maps rendered break tags such as `<next />`/`<br>` as one logical newline. QML also passes
  `selectedText` so the session can repair a drifted RichText selection offset before mutating RAW source.

## Guardrails

- The editor file path must not point at the raw `.wsnbody` XML document.
- LVRS `TextEditor` is a rich-text surface, so loaded source must be projected to editor HTML with explicit line breaks
  before it reaches `filePath`.
- `.wsnbody` parsing and serialization stay in C++ note/editor model code, not QML.
- `openCount` and `lastOpenedAt` updates stay behind the coordinator selected-note bind path; editor QML only observes
  the resulting metadata refresh.
- Format tag allow-list and mutation policy stay in `SetTag`; QML may only pass the requested tag name, current
  cursor/selection metadata, and selected visible text into this session boundary.
- Inline format mutation must not discard existing RAW wrapper tags just because the editor HTML projection no longer
  exposes them as visible text.
- Resource insertion must consume only imported package metadata. Clipboard MIME detection and `.wsresource` package
  persistence stay in `InAppClipboardManager`.

## 한국어

- 이 객체는 선택된 노트를 LVRS `TextEditor`에 연결하기 위한 C++ 세션 경계다.
- `editorFilePath`는 `.wsnbody` 원문이 아니라 editor HTML session file이어야 한다.
- 성공적으로 노트를 mount하면 `ContentsNoteManagementCoordinator`의 selected-note bind 경로로 넘겨
  `openCount`와 `lastOpenedAt` 갱신을 C++ model/session queue가 처리한다.
- `parsedLineCount`는 canonical RAW source line metadata이며 QML이 직접 파일을 읽거나 파싱하지 않게 한다.
  거터의 실제 row 개수는 이 값만 따르며, LVRS rendered wrap-line count를 따르지 않는다.
- `editorViewportWidth`는 QML이 공개 LVRS editor item 폭에서 전달하는 값이며, 이미지 resource frame의 media 영역이
  editor 폭을 채우도록 C++ 렌더러에 전달된다.
- LVRS가 session file 저장을 끝내거나 editor surface revision이 증가하면
  `requestEditorIdleRawPush(...)` / `requestEditorModifiedCountRawPush(...)`가 sync push controller로 전달한다.
  note 이탈 시에는 세션이 같은 controller를 통해 현재 표면을 즉시 RAW로 flush한다.
- editor surface가 cursor/text 활동을 보고하면 `recordEditorUserActivity()`가 active-note idle pull timer를
  다시 시작한다. 5초 동안 조용하면 filesystem RAW를 다시 pull하고, 반환된 `lastModified`가 현재 session context보다
  최신일 때만 editor document를 교체한다. 오래된 pull은 `editorFilesystemPullIgnored(...)`로 무시된다.
- 노트 진입/open 시 filesystem RAW pull 요청은 sync pull controller를 먼저 지나며, 실제 `.wsnbody` 읽기와 editor
  session file mount는 기존 C++ 세션/코디네이터 경로에 남는다.
- 저장 과정에서 timestamp가 찍힌 `.wsnversion` diff가 파일시스템에 쓰이면 `hubFilesystemMutated()`를 전달해
  hub sync가 로컬 변경으로 인식할 수 있게 한다.
- `insertImportedResourcesIntoSource(...)`는 `InAppClipboardManager`가 이미 `.wsresource`로 등록한 metadata만 받아
  canonical RAW `<resource ... />` 참조를 현재 커서/선택 위치에 삽입한다. clipboard MIME 판별과 package
  persistence는 이 세션의 책임이 아니다.
- `reprojectResourceFramesForEditorWidth(...)`는 현재 editor HTML을 canonical source로 복원한 뒤 resource frame이
  있을 때만 새 viewport 폭으로 다시 렌더한다. 기존 `data-frame-display-height`는 초기 auto height로 보존되어,
  리사이즈 중 frame height가 다시 계산되지 않는다.
- `bold`, `italic`, `underline`, `strikethrough`, `highlight`, `break` 같은 포맷 태그는
  `insertFormatTagIntoSource(...)`가 `SetTag`를 통해 RAW source와 editor HTML projection을 함께 계산한다.
  로드된 `.wsnbody` RAW source가 mutation 기준이다. `<next />`/`<br>` 같은 source-level break는 selection 논리
  좌표에서 newline 1글자로 취급한다. LVRS RichText selection 좌표가 밀리면 함께 전달된 selected text로 실제 RAW
  visible 범위를 다시 찾는다.
