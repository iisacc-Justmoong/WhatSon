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
- Carries the loaded note `lastModified` timestamp and RAW source with the editor-file context so RAW pushes can apply
  editor changes as diffs onto the current filesystem body.
- Provides `recordEditorUserActivity()` and `requestActiveNoteIdleRawPull()` for the active-note idle pull loop. The
  idle loop pulls every 5000 ms while the editor surface is quiet, and the session only applies a filesystem body when
  its `lastModified` timestamp is newer than the loaded session timestamp.
- Emits `editorDocumentTextPulled(...)` when a newer filesystem body diff is merged into the live editor document, and
  `editorFilesystemPullIgnored(...)` when an idle pull is stale, inactive, or failed.
- Provides `persistEditorFile(path)` for explicit fallback file-based surface persistence.
- Provides `markEditorSessionFileReadyForRawPush(path)` so QML can declare that LVRS finished reading the freshly
  mounted `.wsnsource` file before sync pushes are accepted. Note entry/open remounts clear any readiness inherited
  from an earlier visit to the same session path.
- Provides `requestEditorIdleRawPush(...)` and `requestEditorModifiedCountRawPush(...)`, which convert the current
  editor document into canonical RAW source inside this session, reject unsafe transient empty editor payloads, then
  forward only validated RAW payloads into `file/sync/WhatSonEditorRawPushController`. Idle push also treats an empty
  rich-text HTML shell as transient when the active RAW source is non-empty.
- Forwards `hubFilesystemMutated()` from the note-management coordinator when a successful editor persistence writes
  a timestamped version diff to `.wsnversion`.
- Provides `insertImportedResourcesIntoSource(...)`, which receives resource package metadata already persisted by
  `InAppClipboardManager`, inserts canonical RAW `<resource ... />` calls at the current editor cursor/selection, and returns
  an editor HTML projection for the live LVRS surface. When an active note is mounted, the same projection is staged to
  the `.wsnsource` session file and older pending push payloads for that file are discarded before control returns to
  QML.
- Provides `reprojectResourceFramesForEditorWidth(...)`, which recovers the current editor document as canonical source
  and re-renders resource frames plus generated callout frame chrome for the current editor viewport width. Resource
  frames preserve each frame's initial `data-frame-display-height`, while callout leading bars are regenerated from the
  current edited content height. QML calls this through a short debounce after text changes so typing inside a wrapped
  callout does not wait for idle persistence but also does not replace the native document on every keystroke.
- Provides `insertFormatTagIntoSource(...)`, which applies a static editor format tag such as `bold`, `italic`,
  `underline`, `strikethrough`, `highlight`, or `break` through `SetTag`, then returns both canonical RAW source and
  an editor HTML projection for the live LVRS surface. `insertStyleTagIntoSource(...)` uses the same selection mapping
  and projection path for the toolbar's `<style>` tag `style` attribute choices. The session keeps the loaded
  `.wsnbody` RAW source as the format mutation basis and maps rendered break tags such as `<next />`/`<br>` as one
  logical newline. QML also passes `selectedText` so the session can repair a drifted RichText selection offset before
  mutating RAW source. Collapsed style selector commands expand to the current non-empty visible source line; an empty
  current line is rejected so the editor does not create and immediately lose a zero-width `<style>` wrapper.
- Provides `handleCalloutBoundaryKeyInSource(...)` for native editor key filters. Backspace at the rendered callout
  content start removes the visual callout wrapper, preserving existing content as plain source and deleting an empty
  callout frame entirely. Enter/Return inside a callout moves the cursor to the next source line outside the wrapper,
  adding that line break when the callout is currently the trailing source node.

## Guardrails

- The editor file path must not point at the raw `.wsnbody` XML document.
- LVRS `TextEditor` is a rich-text surface, so loaded source must be projected to editor HTML with explicit line breaks
  before it reaches `filePath`.
- `.wsnbody` parsing and serialization stay in C++ note/editor model code, not QML.
- `openCount` and `lastOpenedAt` updates stay behind the coordinator selected-note bind path; editor QML only observes
  the resulting metadata refresh.
- Format tag allow-list and mutation policy stay in `SetTag`; QML may only pass the requested tag name, current
  cursor/selection metadata, and selected visible text into this session boundary.
- Callout boundary key behavior stays in this session boundary. QML and event filters may forward raw key/cursor
  metadata, but they must not parse `<callout>` source or mutate wrapper text themselves.
- Inline format mutation must not discard existing RAW wrapper tags just because the editor HTML projection no longer
  exposes them as visible text.
- Resource insertion must consume only imported package metadata. Clipboard MIME detection and `.wsresource` package
  persistence stay in `InAppClipboardManager`.
- Resource insertion and other C++ RAW command results must treat the returned canonical source as authoritative for
  note departure until a newer validated modified-count RAW payload arrives. This prevents a stale session file or
  pre-paste pending push from removing the inserted image when the user moves to another note.
- Resource object placeholder restoration must preserve text on both sides of `QChar::ObjectReplacementCharacter`.
  Qt can serialize text typed immediately below an image into the same block as the image object, and that text is still
  a separate canonical source line.

## 한국어

- 이 객체는 선택된 노트를 LVRS `TextEditor`에 연결하기 위한 C++ 세션 경계다.
- `editorFilePath`는 `.wsnbody` 원문이 아니라 editor HTML session file이어야 한다.
- 성공적으로 노트를 mount하면 `ContentsNoteManagementCoordinator`의 selected-note bind 경로로 넘겨
  `openCount`와 `lastOpenedAt` 갱신을 C++ model/session queue가 처리한다.
- `parsedLineCount`는 canonical RAW source line metadata이며 QML이 직접 파일을 읽거나 파싱하지 않게 한다.
  거터의 실제 row 개수는 이 값만 따르며, LVRS rendered wrap-line count를 따르지 않는다.
- `editorViewportWidth`는 QML이 공개 LVRS editor item 폭에서 전달하는 값이며, 이미지 resource frame의 media 영역과
  콜아웃의 생성형 frame chrome이 현재 editor 폭에 맞게 다시 렌더되도록 C++ 렌더러에 전달된다.
- LVRS가 session file 읽기를 끝내면 QML은 `markEditorSessionFileReadyForRawPush(...)`로 현재 `.wsnsource`를
  push 가능 상태로 표시한다. note를 새로 mount할 때는 같은 session path라도 이전 ready 상태를 재사용하지 않는다.
  이후 session file 저장이 끝나거나 editor surface revision이 증가하면
  `requestEditorIdleRawPush(...)` / `requestEditorModifiedCountRawPush(...)`가 현재 editor document를 C++ 세션 안에서
  canonical RAW source로 변환하고, 빈 rich-text shell 같은 transient idle payload를 거른 뒤 안전한 RAW payload만
  sync push controller로 전달한다. note 이탈 시에는 세션이
  session file fallback이 아니라 active RAW source나 pending RAW payload를 flush한다.
- editor surface가 cursor/text 활동을 보고하면 `recordEditorUserActivity()`가 active-note idle pull timer를
  다시 시작한다. 5초 동안 조용하면 filesystem RAW를 다시 pull하고, 반환된 `lastModified`가 현재 session context보다
  최신일 때만 loaded RAW base에서 filesystem RAW로의 diff를 active RAW source에 병합한다. 오래된 pull이나
  적용 불가능한 diff는 `editorFilesystemPullIgnored(...)`로 무시된다.
- 노트 진입/open 시 filesystem RAW pull 요청은 sync pull controller를 먼저 지나며, 실제 `.wsnbody` 읽기와 editor
  session file mount는 기존 C++ 세션/코디네이터 경로에 남는다.
- 저장 과정에서 timestamp가 찍힌 `.wsnversion` diff가 파일시스템에 쓰이면 `hubFilesystemMutated()`를 전달해
  hub sync가 로컬 변경으로 인식할 수 있게 한다.
- `insertImportedResourcesIntoSource(...)`는 `InAppClipboardManager`가 이미 `.wsresource`로 등록한 metadata만 받아
  canonical RAW `<resource ... />` 참조를 현재 커서/선택 위치에 삽입한다. clipboard MIME 판별과 package
  persistence는 이 세션의 책임이 아니다. active note가 mount된 상태에서는 반환할 editor HTML projection을
  `.wsnsource` session file에 즉시 기록하고, 같은 파일의 오래된 pending push를 폐기한다. 이후 사용자가 노트를
  이동해도 더 최신 modified-count payload가 없다면 note-departure flush는 이 canonical source를 저장 기준으로
  사용한다.
- 이미지 object placeholder 복원은 `QChar::ObjectReplacementCharacter` 앞뒤 텍스트를 보존해야 한다. Qt가 이미지
  바로 아래에 입력한 텍스트를 image object와 같은 block으로 직렬화해도, 그 텍스트는 별도 canonical source line이다.
- `reprojectResourceFramesForEditorWidth(...)`는 현재 editor HTML을 canonical source로 복원한 뒤 resource frame과
  callout frame chrome이 있을 때 새 viewport 폭으로 다시 렌더한다. resource의 기존 `data-frame-display-height`는
  초기 auto height로 보존되고, callout의 좌측 막대는 현재 편집된 콘텐츠의 wrap 높이로 재생성된다. QML은 텍스트
  변경 후 짧은 debounce를 두고 이 함수를 호출해 native document를 매 keystroke마다 교체하지 않는다.
- `bold`, `italic`, `underline`, `strikethrough`, `highlight`, `break` 같은 포맷 태그는
  `insertFormatTagIntoSource(...)`가 `SetTag`를 통해 RAW source와 editor HTML projection을 함께 계산한다.
  toolbar style selector의 `<style>` tag `style` attribute 선택은 `insertStyleTagIntoSource(...)`가 같은 selection
  mapping과 projection 경로로 처리한다.
  로드된 `.wsnbody` RAW source가 mutation 기준이다. `<next />`/`<br>` 같은 source-level break는 selection 논리
  좌표에서 newline 1글자로 취급한다. LVRS RichText selection 좌표가 밀리면 함께 전달된 selected text로 실제 RAW
  visible 범위를 다시 찾는다. style selector 명령은 selection이 없을 때 현재 non-empty visible source line 전체를
  대상으로 하며, 빈 줄에서는 zero-width `<style>` wrapper를 만들지 않는다.
- `handleCalloutBoundaryKeyInSource(...)`는 콜아웃 내부 key boundary를 처리한다. 콜아웃 content 시작점의
  Backspace는 콜아웃 wrapper를 제거하고 내용은 일반 source로 남기며, 빈 콜아웃 frame은 줄째 삭제한다.
  콜아웃 내부 Enter/Return은 콜아웃을 유지한 채 커서를 닫는 태그 바깥 다음 source line으로 이동시키고, 뒤에
  줄이 없으면 새 줄을 추가한다.
