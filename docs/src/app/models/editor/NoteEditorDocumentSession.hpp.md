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
  render editor-width media inside the resource-frame container and generated callout chrome can match wrapped text.
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
- Provides `markEditorSessionFileReadyForRawPush(path)` so QML can acknowledge the LVRS `readFinished(path)` event for
  the currently mounted session file before idle or modified-count push triggers are accepted.
- Provides `requestEditorModifiedCountRawPush(...)`, which accepts the fresh LVRS `textEdited(text)` payload after the
  current session file has been marked ready, converts it to RAW immediately, and writes the selected `.wsnbody` without
  waiting for the idle sync path.
- Provides `requestEditorIdleRawPush(...)` as a fallback for LVRS session-file sync completion. Idle payloads are ignored
  when direct RAW input has already advanced the active source.
- Forwards `hubFilesystemMutated()` from the note-management coordinator when a successful editor persistence writes
  a timestamped version diff to `.wsnversion`.
- Provides `insertImportedResourcesIntoSource(...)`, which receives resource package metadata already persisted by
  `InAppClipboardManager`, recovers the current editor snapshot as RAW source, inserts canonical RAW `<resource ... />`
  calls at the current editor cursor without replacing the surrounding selection, updates the mounted session file,
  queues `.wsnbody` persistence, and returns an editor HTML projection for the live LVRS surface.
- Provides `reprojectResourceFramesForEditorWidth(...)`, which recovers the current editor document as canonical source
  and re-renders resource frames plus generated callout frame chrome for the current editor viewport width. Resource
  frames preserve each frame's initial `data-frame-display-height`, while callout leading bars are regenerated from the
  current edited content height. QML calls this through a short debounce after text changes so typing inside a wrapped
  callout does not wait for idle persistence but also does not replace the native document on every keystroke.
- Provides `insertFormatTagIntoSource(...)`, which applies a static editor format tag such as `bold`, `italic`,
  `callout`, `header`, `subheader`, or `resource`, then returns both canonical source and an editor HTML projection for
  the live LVRS surface. The session keeps the loaded `.wsnbody` RAW source as the format mutation basis and maps
  rendered break tags such as `<next />`/`<br>` as one logical newline. QML also passes `selectedText` so the session can
  repair a drifted RichText selection offset before mutating RAW source. Empty callout insertion returns the LVRS
  rich-text cursor at the rendered callout content start, after generated frame chrome images, so the caret lands inside
  the callout rather than on the neighboring source line.
- Provides `handleCalloutBoundaryKeyInSource(...)` for native editor key filters. Backspace at the rendered callout
  content start removes the visual callout wrapper, preserving existing content as plain source and deleting an empty
  callout frame entirely. That content-start test uses the loaded RAW source and skips generated frame chrome plus
  renderer-only line characters in LVRS rich-text coordinates. Enter/Return inside a callout moves the cursor to the
  next source line outside the wrapper, adding that line break when the callout is currently the trailing source node.

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
- Resource paste treats the current LVRS editor snapshot as authoritative for the insertion source. A loaded active source
  cache must not overwrite text typed immediately before `Cmd+V`.
- Resource paste discards pending RAW pushes for the active session file after the canonical tag insertion so a queued
  older editor snapshot cannot remove the just-inserted `<resource ... />` line.

## 한국어

- 이 객체는 선택된 노트를 LVRS `TextEditor`에 연결하기 위한 C++ 세션 경계다.
- `editorFilePath`는 `.wsnbody` 원문이 아니라 editor HTML session file이어야 한다.
- 성공적으로 노트를 mount하면 `ContentsNoteManagementCoordinator`의 selected-note bind 경로로 넘겨
  `openCount`와 `lastOpenedAt` 갱신을 C++ model/session queue가 처리한다.
- `parsedLineCount`는 canonical RAW source line metadata이며 QML이 직접 파일을 읽거나 파싱하지 않게 한다.
  거터의 실제 row 개수는 이 값만 따르며, LVRS rendered wrap-line count를 따르지 않는다.
- `editorViewportWidth`는 QML이 공개 LVRS editor item 폭에서 전달하는 값이며, 이미지 resource frame의 media 영역과
  콜아웃의 생성형 frame chrome이 현재 editor 폭에 맞게 다시 렌더되도록 C++ 렌더러에 전달된다.
- LVRS가 현재 session file의 `readFinished(path)`를 낸 뒤에만
  `markEditorSessionFileReadyForRawPush(path)`가 해당 파일을 write 가능한 editor 세션으로 승격한다. 앱 시작 직후의
  blank editor나 이전 파일의 늦은 read-finished 이벤트는 새로 선택된 노트를 덮어쓰는 RAW push를 열지 못한다.
- LVRS `textEdited(text)` 입력은 `requestEditorModifiedCountRawPush(...)`로 들어와 즉시 RAW로 변환되고 선택된
  `.wsnbody`에 기록된다. `syncFinished(path)`는 fallback idle trigger일 뿐이며, direct RAW 입력보다 오래된 payload는
  active source로 승격되지 않는다. note 이탈 시에는 direct RAW 저장이 없을 때만 fallback flush를 수행한다.
- editor surface가 cursor/text 활동을 보고하면 `recordEditorUserActivity()`가 active-note idle pull timer를
  다시 시작한다. 5초 동안 조용하면 filesystem RAW를 다시 pull하고, 반환된 `lastModified`가 현재 session context보다
  최신일 때만 editor document를 교체한다. 오래된 pull은 `editorFilesystemPullIgnored(...)`로 무시된다.
- 노트 진입/open 시 filesystem RAW pull 요청은 sync pull controller를 먼저 지나며, 실제 `.wsnbody` 읽기와 editor
  session file mount는 기존 C++ 세션/코디네이터 경로에 남는다.
- 저장 과정에서 timestamp가 찍힌 `.wsnversion` diff가 파일시스템에 쓰이면 `hubFilesystemMutated()`를 전달해
  hub sync가 로컬 변경으로 인식할 수 있게 한다.
- `insertImportedResourcesIntoSource(...)`는 `InAppClipboardManager`가 이미 `.wsresource`로 등록한 metadata만 받아
  현재 LVRS editor snapshot을 RAW source로 복원한 뒤 canonical RAW `<resource ... />` 참조를 현재 커서/선택
  위치에 삽입한다. 삽입 성공 시 mounted session file과 `.wsnbody` persistence를 갱신하고, 같은 session file의
  오래된 pending RAW push를 버린다. clipboard MIME 판별과 package persistence는 이 세션의 책임이 아니다.
- `reprojectResourceFramesForEditorWidth(...)`는 현재 editor HTML을 canonical source로 복원한 뒤 resource frame과
  callout frame chrome이 있을 때 새 viewport 폭으로 다시 렌더한다. resource의 기존 `data-frame-display-height`는
  초기 auto height로 보존되고, callout의 좌측 막대는 현재 편집된 콘텐츠의 wrap 높이로 재생성된다. QML은 텍스트
  변경 후 짧은 debounce를 두고 이 함수를 호출해 native document를 매 keystroke마다 교체하지 않는다.
  `insertFormatTagIntoSource(...)`가 `SetTag`를 통해 RAW source와 editor HTML projection을 함께 계산한다.
  로드된 `.wsnbody` RAW source가 mutation 기준이다. `<next />`/`<br>` 같은 source-level break는 selection 논리
  좌표에서 newline 1글자로 취급한다. LVRS RichText selection 좌표가 밀리면 함께 전달된 selected text로 실제 RAW
  visible 범위를 다시 찾는다. 빈 callout 삽입은 생성된 frame chrome image를 건너뛴 LVRS rich-text content 시작
  좌표를 반환해 커서가 주변 줄이 아니라 새 callout 내부에 놓이게 한다.
- `handleCalloutBoundaryKeyInSource(...)`는 콜아웃 내부 key boundary를 처리한다. 콜아웃 content 시작점의
  Backspace는 콜아웃 wrapper를 제거하고 내용은 일반 source로 남기며, 빈 콜아웃 frame은 줄째 삭제한다. 이
  content 시작점 판정은 로드된 RAW source를 기준으로 하며 LVRS rich-text 좌표의 생성 frame chrome과 renderer 전용
  줄 문자를 건너뛴다. 콜아웃 내부 Enter/Return은 콜아웃을 유지한 채 커서를 닫는 태그 바깥 다음 source line으로
  이동시키고, 뒤에 줄이 없으면 새 줄을 추가한다.
