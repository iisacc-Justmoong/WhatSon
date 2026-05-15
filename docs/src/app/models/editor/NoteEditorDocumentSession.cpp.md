# `src/app/models/editor/NoteEditorDocumentSession.cpp`

## Responsibility

Implements the active note editor document session.

## Runtime Flow

1. Active-note changes are read from `NoteActiveStateTracker`.
2. Note entry/open pull requests are routed through `file/sync/WhatSonEditorRawPullController`, whose callback delegates
   to `ContentsNoteManagementCoordinator` to load the selected note body as canonical editor-facing RAW source.
3. The source is projected into editor HTML and written to a cache/session `.wsnsource` file so LVRS
   `TextEditor` receives explicit rich-text line breaks.
4. After the session file is mounted, the session binds that selected note through
   `ContentsNoteManagementCoordinator::bindSelectedNote(...)` so the same coordinator queue advances `openCount` and
   `lastOpenedAt` without letting the editor surface implement statistics policy.
5. QML binds that session file into LVRS `TextEditor.filePath`, keeps the parsed source line count as session metadata,
   binds the current editor viewport width into the session, and uses the parsed line count as the gutter delegate
   count. The sibling editor supplies only rendered start positions for those parsed source lines.
6. When LVRS emits `syncFinished(path)`, QML sends the editor document text to
   `requestEditorIdleRawPush(...)`; when the editor surface modified count increases, QML sends the same surface
   payload to `requestEditorModifiedCountRawPush(...)`. Both routes pass through
   `WhatSonEditorRawPushController`, then the session converts the editor document HTML back into canonical source text
   and delegates persistence through `ContentsNoteManagementCoordinator` so `.wsnbody` is reserialized and
   `parsedLineCount` is refreshed.
   If that persistence writes a timestamped `.wsnversion` diff, the coordinator signal is forwarded as
   `hubFilesystemMutated()` for the composition-root hub-sync wiring.
   Before note context changes or clears, the session asks the same push controller to flush the active surface.
7. Editor format shortcuts call `insertFormatTagIntoSource(...)`; the session mutates the loaded `.wsnbody` RAW source,
   maps the rendered selection to RAW visible-character positions, applies `SetTag`, returns a fresh editor HTML
   projection, and maps the source cursor back to the rendered editor cursor position.
8. Clipboard resource paste calls `insertImportedResourcesIntoSource(...)` only after `InAppClipboardManager` has persisted
   the resource package. The session inserts RAW resource tags and returns an editor HTML projection that renders each
   standalone resource source line as a resource frame.

## Guardrails

- The session creates blank/editor session files outside the note package.
- The session keeps a file-path-to-note-context map so late sync signals from a previous editor file still persist to
  the note that originally owned that file.
- Note entry/open RAW pulls are routed through `file/sync/WhatSonEditorRawPullController`; the session remains the
  load callback and editor projection owner.
- Idle, modified-count, and note-departure RAW pushes are routed through
  `file/sync/WhatSonEditorRawPushController`; the session remains the conversion/write callback owner.
- Re-selecting the same note keeps the existing session file intact, so unsaved editor state is not overwritten
  by a redundant body reload.
- The open-count update is a selected-note bind side effect owned by `ContentsNoteManagementCoordinator`; editor QML must
  not write statistic fields directly.
- The session computes parsed RAW source line count in C++ so QML gutter code does not read or parse note files. The
  gutter uses that metadata as its row count and must not derive row count from LVRS rendered wrap-line geometry.
- Imported-resource insertion consumes metadata returned by `InAppClipboardManager`; it must not inspect MIME data or create
  resource packages itself.
- Standalone `<resource ... />` source lines are atomic editor slots. The session renders them with
  `component/ResourceImageFrame` and wraps that frame in `whatson-resource-source` markers so the persistence boundary can
  recover the exact canonical source tag. Image resources whose package asset resolves from the active note/hub context
  render as `<img src="file://...">` inside the Figma `292:50` resource frame; unresolved or non-image resources still
  render as a visible resource frame with the stored resource reference.
- Resource frame projection uses `editorViewportWidth` from QML as the media raster's intrinsic width, while preserving
  a structured `width="100%"` frame, `height:auto` media, and dynamic centered image placement in the emitted HTML.
  The first projection records `data-frame-display-height`; `reprojectResourceFramesForEditorWidth(...)` parses that
  existing marker and reuses it as the locked frame height, so a window resize changes frame width and x-axis centering
  without changing the initial auto height.
- When Qt serializes the rich editor document and strips those HTML markers, image frames remain as rich-text object
  replacement characters. `persistEditorFile(...)` restores those object placeholders from the active canonical source
  resource lines before delegating `.wsnbody` persistence, preserving the resource reference across real editor save
  round-trips.
- If Backspace/Delete removes the rich-text object, the missing object means the resource frame was deleted; no
  header/footer chrome cleanup path is kept in the current image-only frame contract.
- After a successful editor persistence callback, the session canonicalizes the persisted source through the same
  `.wsnbody` serializer/read-back boundary before refreshing `parsedLineCount`. This keeps deleted atomic resource
  objects from leaving a transient trailing editor line in the gutter contract.
- The session does not decide hub-sync policy itself; it only forwards the coordinator's version-diff filesystem
  mutation signal so `main.cpp` can wire that signal into `WhatSonHubSyncController`.
- Static format tags are inserted by `SetTag` through `insertFormatTagIntoSource(...)`. QML supplies only the tag name,
  current editor document text, cursor, selection length, and selected visible text; source mutation, same-tag toggle
  removal, projection, and line-count refresh stay here. The session treats the loaded `.wsnbody` RAW source as the
  format mutation basis so a lossy RichText projection cannot drop blank source rows before selection mapping.
  Source-level rendered break tags such as `<next />` and `<br>` count as one logical newline while selection is
  mapped. If the RichText selection offset has drifted, the session compares that selected text with the visible source
  span and repairs the source range before calling `SetTag`.
- It must not expose the raw XML body file as the editor file path.

## 한국어

- 이 구현은 `.wsnbody` XML을 그대로 에디터에 띄우지 않는다.
- 선택된 노트의 본문을 RAW source로 파싱한 뒤 LVRS `TextEditor`가 줄바꿈을 보존해 렌더할 수 있는 editor HTML
  session file로 투영하고, LVRS 저장 이벤트는 다시 canonical source로 복원해 `.wsnbody` 직렬화 경로로 연결한다.
- parsed RAW source line count는 이 C++ 세션이 계산한다. 거터 표시 row 개수는 이 metadata만 따른다.
  paragraph wrap으로 생긴 rendered row count는 거터 row count에 참여하지 않는다.
- QML은 공개 LVRS editor item 폭을 `editorViewportWidth`로 전달한다. 이 값은 resource frame media raster의
  intrinsic width가 되어 Qt rich text에서 프레임이 editor 폭을 채우도록 만들고, 실제 이미지 표시 박스는 그 frame
  폭 안에서 다시 중앙 정렬된다.
- clipboard resource paste는 `InAppClipboardManager`가 `.wsresource` package를 먼저 만든 뒤 이 세션의
  `insertImportedResourcesIntoSource(...)`로 들어온다. 세션은 본문 RAW source에 `<resource ... />` 참조를
  삽입하고 editor HTML projection을 반환한다. standalone resource source line은 editor HTML에서
  `component/ResourceImageFrame`을 통해 Figma `292:50` 형태의 `whatson-resource-frame`으로 렌더링되고, 이미지 package
  asset을 active note/hub 기준으로 찾을 수 있으면 `file://` 이미지로 표시된다. 저장 시에는
  `whatson-resource-source` marker가 다시 canonical `<resource ... />` source tag로 복구된다. Qt RichText 직렬화가
  marker를 제거한 경우에도 `persistEditorFile(...)`은 active canonical source의 resource line과 이미지 object
  placeholder를 기준으로 `<resource ... />`를 복원한다. Backspace/Delete 뒤 이미지 object가 사라진 경우에는
  resource frame이 삭제된 것으로 본다. persistence 성공 콜백 뒤에는 `.wsnbody` serializer/read-back 경계의
  canonical source로 parsed line count를 다시 맞춰, 삭제된 atomic frame 뒤의 임시 trailing line이 거터 계약에
  남지 않게 한다.
- editor viewport 폭이 바뀌면 `reprojectResourceFramesForEditorWidth(...)`가 현재 editor HTML을 source로 복원하고,
  resource frame이 있는 경우에만 새 폭으로 frame preview를 다시 만든다. 이때 기존
  `data-frame-display-height`를 읽어 frame height를 초기 auto 값으로 고정하므로, resize는 frame 폭과 x축 중앙
  offset만 바꾸고 높이는 바꾸지 않는다.
- 포맷 단축키는 이 세션의 `insertFormatTagIntoSource(...)`로 들어오며, 로드된 `.wsnbody` RAW source를 기준으로
  mutation한다. editor RichText projection이 빈 source row를 손실해도 selection 좌표는 `.wsnbody` source의 논리
  row를 기준으로 변환한다. `<next />`와 `<br>` 같은 source-level rendered break는 selection 논리 좌표에서 newline
  1글자로 센다. 좌표가 selected text와 맞지 않으면 실제 visible source에서 selected text 위치를 다시 찾아 paragraph
  밖으로 wrapper가 새는 것을 막는다.
  같은 태그가 정확히 감싼 selection이면 `SetTag`가 wrapper를 제거하는 toggle 결과를 반환한다.
