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
5. The same mounted note becomes the active idle-pull target. User activity reported by the editor surface restarts the
   pull timer; once the note remains idle for 5000 ms, `WhatSonEditorRawPullController` queues an `idle` pull.
   The session compares the returned filesystem `lastModified` timestamp against the session-file context timestamp and
   only processes the pull when the filesystem copy is newer. The newer filesystem body is applied as a diff from the
   loaded RAW base onto the active RAW source, so local editor mutations and remote filesystem mutations can coexist
   without a full-body replacement.
6. QML binds that session file into LVRS `TextEditor.filePath`, keeps the parsed source line count as session metadata,
   binds the current editor viewport width into the session, and uses the parsed line count as the gutter delegate
   count. The sibling editor supplies only rendered start positions for those parsed source lines.
7. When LVRS emits `syncFinished(path)`, QML asks `requestEditorIdleRawPush(...)` to inspect the current editor
   document; when the editor surface modified count increases, QML asks `requestEditorModifiedCountRawPush(...)` to do
   the same. The session first verifies that `readFinished(path)` marked the freshly mounted `.wsnsource` file ready,
   converts the editor document into canonical RAW source, rejects transient empty editor payloads over a non-empty
   active RAW source, and only then queues `WhatSonEditorRawPushController`. Empty Qt rich-text shells such as an empty
   paragraph HTML document are treated as transient for idle sync because they can be emitted while LVRS is still
   settling a remounted session file. The push controller stores that verified RAW source, not the editor
   HTML/session-file snapshot. Persistence then writes the queued RAW source through
   `ContentsNoteManagementCoordinator`, which applies the editor RAW diff from the loaded RAW base onto the current
   filesystem RAW body before `.wsnbody` is reserialized and `parsedLineCount` is refreshed.
   If that persistence writes a timestamped `.wsnversion` diff, the coordinator signal is forwarded as
   `hubFilesystemMutated()` for the composition-root hub-sync wiring.
   Before note context changes or clears, the session flushes the active RAW source or the controller's pending RAW
   payload; it no longer falls back to re-reading an arbitrary session-file snapshot for sync.
8. Editor format shortcuts call `insertFormatTagIntoSource(...)`; the toolbar style selector calls
   `insertStyleTagIntoSource(...)` for the `<style>` tag `style` attribute values, and the toolbar font selector calls
   `insertStyleFontTagIntoSource(...)` for the `<style>` tag `font` attribute value. The session mutates the loaded
   `.wsnbody` RAW source, maps the rendered selection to RAW visible-character positions, applies `SetTag`, returns a
   fresh editor HTML projection, and maps the source cursor back to the rendered editor cursor position. A collapsed
   style-selector command expands to the current non-empty visible source line instead of inserting an empty
   `<style></style>` wrapper that would be lost on the next editor round-trip. The font-size toolbar command follows
   the same path with a validated `<style size="...">` wrapper. For an active note, the style command
   also writes that fresh projection to the mounted `.wsnsource` session file and enqueues the canonical `<style ...>`
   RAW body persist immediately, so neither LVRS file sync nor a later filesystem pull can restore the pre-style
   paragraph snapshot during the idle RAW-push window. Source-visible text mapping treats `<style>` tags as invisible
   wrappers, matching bold/link/tag behavior, so a later LVRS-normalized rich-text payload with the same visible text
   cannot flatten the active styled RAW source back into a plain paragraph. If a later plain editor push edits visible
   text inside an existing `<style>` wrapper or at its content boundary, the session applies that single visible edit to
   the active RAW wrapper instead of accepting renderer-shifted or missing style marker positions. Ordinary characters
   remain inside the wrapper, while newline remains the explicit wrapper exit.
   The `bold` format command is represented as `<style weight="900">...</style>` instead of a new `<bold>` wrapper, so
   the command binds to the same style `weight` axis used by the toolbar and does not create `<bold>`/`<style>` overlap.
   The toolbar also asks `toolbarStyleContextAtCursor(...)` for the current cursor context; this maps the LVRS editor
   cursor back to the active RAW source and reports the innermost `<style ...>` wrapper's `style`, `font`, `size`,
   `weight`, and `height` values for display. The same query reports whether the cursor is inside active inline format
   wrappers for legacy `bold`, `italic`, `underline`, `strikethrough`, and `highlight`, and it treats an explicit style
   `weight` of `700+` as the active bold binding. This allows toolbar buttons to mirror the current RAW context without
   parsing source in QML. Outside a style wrapper it returns the Body/Pretendard defaults.
9. Clipboard resource paste calls `insertImportedResourcesIntoSource(...)` only after `InAppClipboardManager` has persisted
   the resource package. The session inserts RAW resource tags and returns an editor HTML projection that renders each
   standalone resource source line as a resource frame. For an active note, that command result is also written to the
   mounted `.wsnsource` session file immediately, any older pending RAW push for that file is discarded, and the new
   active source is marked as the note-departure save baseline until a later modified-count editor payload supersedes it.
10. Editor key filters call `handleCalloutBoundaryKeyInSource(...)` before native text handling only for plain
    Backspace on callout boundaries. The session maps the rendered cursor back to loaded RAW source and unwraps or
    removes a callout at its content start. The same editor key filter calls `handleStyleBoundaryKeyInSource(...)` for
    style Return/Enter only, so Enter exits the style wrapper while ordinary typing continues at the style content end.
    If the style closing tag is followed only by trailing newlines, the helper reports unhandled and lets native
    `TextEdit` extend the empty line.

## Guardrails

- The session creates blank/editor session files outside the note package.
- The session keeps a file-path-to-note-context map so late sync signals from a previous editor file still persist to
  the note that originally owned that file.
- Note entry/open RAW pulls are routed through `file/sync/WhatSonEditorRawPullController`; the session remains the
  load callback and editor projection owner.
- The loaded note's `lastModified` timestamp and RAW source are kept with the editor session-file context. Later RAW
  pushes pass that base source into the note-management queue so the editor change can be applied as a diff onto the
  current filesystem body instead of replacing the whole note package body.
- Fresh note entry/open mounts reset RAW-push readiness even when the same `.wsnsource` path was marked ready in an
  earlier visit. Only a new `readFinished(path)` may re-enable push for that remounted file. Idle pulls that update the
  already-mounted active document keep readiness because QML receives a programmatic document replacement instead of a
  new file read.
- Active-note idle RAW pulls reuse that same timestamp context in the opposite direction: a filesystem pull whose
  `lastModified` is not strictly newer is ignored and reported through `editorFilesystemPullIgnored(...)`; a newer pull
  applies the filesystem diff onto the active RAW source, rewrites the session file with the merged result, emits
  `editorDocumentTextPulled(...)`, and updates the stored filesystem base source/timestamp.
- Idle, modified-count, and note-departure RAW pushes are routed through
  `file/sync/WhatSonEditorRawPushController` only after this session has converted the live editor document into
  validated RAW source. The controller orders RAW payloads; it does not own editor HTML or reread the mounted session
  file as sync truth.
- Re-selecting the same note keeps the existing session file intact, so unsaved editor state is not overwritten
  by a redundant body reload.
- The open-count update is a selected-note bind side effect owned by `ContentsNoteManagementCoordinator`; editor QML must
  not write statistic fields directly.
- The session computes parsed RAW source line count in C++ so QML gutter code does not read or parse note files. The
  gutter uses that metadata as its row count and must not derive row count from LVRS rendered wrap-line geometry.
- Imported-resource insertion consumes metadata returned by `InAppClipboardManager`; it must not inspect MIME data or create
  resource packages itself.
- Imported-resource insertion is an authoritative source mutation. It stages the projected editor HTML into the mounted
  session file and invalidates stale pending push payloads so moving to another note cannot persist the pre-paste editor
  snapshot over the new `<resource ... />` source line.
- Standalone `<resource ... />` source lines are atomic editor slots. The session renders them with
  `component/ResourceImageFrame` and wraps that frame in `whatson-resource-source` markers so the persistence boundary can
  recover the exact canonical source tag. Image resources whose package asset resolves from the active note/hub context
  render as a single `<img src="file://...">` carrying the Figma `292:50` resource frame; unresolved or non-image resources still
  render as a visible resource frame with the stored resource reference.
- Resource frame projection uses `editorViewportWidth` from QML as the media raster's intrinsic width, while preserving
  explicit frame `width`/`height` attributes, `vertical-align:top`, and dynamic centered image placement in the emitted
  HTML.
  The first projection records `data-frame-display-height`; `reprojectResourceFramesForEditorWidth(...)` parses that
  existing marker and reuses it as the locked frame height, so a window resize changes frame width and x-axis centering
  without changing the initial auto height. The same reproject hook also recognizes callout frames and regenerates their
  leading-bar image from the currently edited content. QML schedules that hook through a short debounce after text
  changes, so wrapped callout chrome updates without waiting for idle persistence and without replacing the native
  document on every keystroke.
- When Qt serializes the rich editor document and strips those HTML markers, image frames remain as rich-text object
  replacement characters. `persistEditorFile(...)` restores those object placeholders from the active canonical source
  resource lines before delegating `.wsnbody` persistence, preserving the resource reference across real editor save
  round-trips. If text typed immediately below the image is serialized into the same rich-text block as the object
  replacement character, the restore path splits that line into the resource source line plus the trailing typed text
  line instead of replacing the whole block with only the resource.
- If Backspace/Delete removes the rich-text object, the missing object means the resource frame was deleted; no
  header/footer chrome cleanup path is kept in the current image-only frame contract.
- After a successful editor persistence callback, the session canonicalizes the persisted source through the same
  `.wsnbody` serializer/read-back boundary before refreshing `parsedLineCount`. This keeps deleted atomic resource
  objects from leaving a transient trailing editor line in the gutter contract.
- Before note departure, an imported-resource command result or other C++ RAW command result that has not yet been
  superseded by a validated modified-count RAW payload is persisted from the active canonical source, not from the
  possibly stale mounted session file and not from an older queued idle/modified push.
- Idle RAW push must not persist an empty rich-text editor shell over a non-empty active RAW source. This guards the
  observed race where a remounted note path and a still-blank LVRS document snapshot briefly overlap.
- The session does not decide hub-sync policy itself; it only forwards the coordinator's version-diff filesystem
  mutation signal so `main.cpp` can wire that signal into `WhatSonHubSyncController`.
- Static format tags are inserted by `SetTag` through `insertFormatTagIntoSource(...)`, style selector values are
  inserted through `insertStyleTagIntoSource(...)`, and font selector values are inserted through
  `insertStyleFontTagIntoSource(...)`. QML supplies only the tag/style/font name, current editor document text,
  cursor, selection length, and selected visible text; source mutation, same-tag toggle removal, projection, and
  line-count refresh stay here. The session treats the loaded `.wsnbody` RAW source as the format mutation basis so a
  lossy RichText projection cannot drop blank source rows before selection mapping.
  Source-level rendered break tags such as `<next />` and `<br>` count as one logical newline while selection is
  mapped. If the RichText selection offset has drifted, the session compares that selected text with the visible source
  span and repairs the source range before calling `SetTag`. Style selector commands with no selection expand to the
  current non-empty visible source line; empty lines are rejected instead of producing zero-width wrappers. Font-size
  toolbar commands use the same expansion path and author a validated `size` attribute. Valid style
  selector mutations are staged into the active editor session file and persisted into the note body before QML replaces
  the live LVRS text, matching the resource insertion race guard while making `.wsnbody` authoritative immediately.
  The same source-visible mapper hides `<style>` wrappers when validating later raw pushes, preserving the active style
  source if LVRS returns equivalent visible text without the original style markers. Plain editor raw pushes that add,
  delete, or replace visible text inside an existing style wrapper are merged against the active RAW source so the
  pre-existing `<style>` wrapper does not disappear unless a style command explicitly mutates it. Ordinary typed text at
  the style end is preserved inside the wrapper; Return/Enter is the explicit style exit and is placed outside the
  wrapper.
- Cursor-driven toolbar display values are queried through `toolbarStyleContextAtCursor(...)`. This method is read-only:
  it parses only the active editor RAW source around the mapped cursor position and never mutates the session file,
  `.wsnbody`, or pending push state.
- Callout Backspace boundary keys are also source mutations owned by this class. Backspace at the callout content start
  removes only the `<callout>` wrapper when content exists, or removes the whole empty callout source line when it does
  not. Because the visual callout frame uses one generated chrome object for the leading bar, this class converts
  between decorated TextEdit offsets and source-visible offsets before applying explicit boundary rules.
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
  placeholder를 기준으로 `<resource ... />`를 복원한다. 이미지 바로 아래에 입력한 첫 텍스트가 object replacement와
  같은 rich-text block으로 직렬화되어도, 복원 경로는 그 줄을 resource source line과 뒤따르는 텍스트 source line으로
  분리해 보존한다. Backspace/Delete 뒤 이미지 object가 사라진 경우에는
  resource frame이 삭제된 것으로 본다. persistence 성공 콜백 뒤에는 `.wsnbody` serializer/read-back 경계의
  canonical source로 parsed line count를 다시 맞춰, 삭제된 atomic frame 뒤의 임시 trailing line이 거터 계약에
  남지 않게 한다. active note에서 이미지 resource 삽입 결과는 즉시 mounted `.wsnsource`에도 기록되고, 그 파일에
  남아 있던 오래된 pending push는 폐기된다. 이후 노트를 떠날 때 아직 더 최신 modified-count payload가 없다면
  note-departure flush는 stale session file이 아니라 이 active canonical source를 `.wsnbody`에 저장한다.
- editor viewport 폭이 바뀌거나 callout 텍스트가 편집되면 QML이 짧은 debounce 뒤
  `reprojectResourceFramesForEditorWidth(...)`를 호출한다. 이 함수는 현재 editor HTML을 source로 복원하고, resource
  frame 또는 callout frame chrome이 있는 경우 새 폭으로 projection을 다시 만든다. resource는 기존
  `data-frame-display-height`를 읽어 frame height를 초기 auto 값으로 고정하므로, resize는 frame 폭과 x축 중앙
  offset만 바꾸고 높이는 바꾸지 않는다. callout은 현재 편집된 content의 wrap 높이에 맞춰 좌측 막대 이미지를 다시
  생성한다.
- 포맷 단축키는 이 세션의 `insertFormatTagIntoSource(...)`로 들어오며, toolbar style selector는
  `insertStyleTagIntoSource(...)`로 `<style>` tag `style` attribute 값을 전달하고, toolbar font selector는
  `insertStyleFontTagIntoSource(...)`로 `<style>` tag `font` attribute 값을 전달한다. 세 경로 모두 로드된
  `.wsnbody` RAW source를 기준으로 mutation한다. editor RichText projection이 빈 source row를 손실해도 selection 좌표는 `.wsnbody` source의 논리
  row를 기준으로 변환한다. `<next />`와 `<br>` 같은 source-level rendered break는 selection 논리 좌표에서 newline
  1글자로 센다. 좌표가 selected text와 맞지 않으면 실제 visible source에서 selected text 위치를 다시 찾아 paragraph
  밖으로 wrapper가 새는 것을 막는다.
  style selector 명령은 selection이 없을 때 현재 non-empty visible source line 전체로 확장하며, 빈 줄에서는
  즉시 사라지는 zero-width `<style>` wrapper를 만들지 않는다. font-size toolbar 명령도 같은 경로로 양의 정수
  `size` attribute를 작성한다. 유효한 style selector mutation은 active editor
  session file에 즉시 stage되고 `.wsnbody` RAW body에도 바로 persist되어 LVRS idle sync가 이전 paragraph snapshot으로
  되돌리지 못하게 한다. 이후 marker 없는 plain editor payload가 들어와도 style wrapper 안쪽의 단일 visible edit는
  active RAW wrapper에 반영하므로, 이어 입력이나 Backspace만으로 `<style>` tag가 사라지지 않는다. styled rendered
  text 끝에서 이어 입력한 일반 텍스트는 wrapper 안에 유지하고, plain Enter/Return만 wrapper 밖 다음 source line으로
  나가는 boundary edit로 처리한다.
  toolbar 표시값은 `toolbarStyleContextAtCursor(...)`가 제공한다. 이 함수는 현재 cursor를 RAW source 좌표로 되돌린
  뒤 가장 안쪽의 `<style ...>` wrapper를 찾아 `style`, `font`, `size`, `weight`, `height` 표시값을 반환하며, wrapper
  밖에서는 Body/Pretendard 기본값을 반환한다. 이는 읽기 전용 조회이며 source mutation이나 persistence를 수행하지
  않는다.
  같은 태그가 정확히 감싼 selection이면 `SetTag`가 wrapper를 제거하는 toggle 결과를 반환한다.
- 콜아웃 Backspace 경계 키도 이 세션에서 처리한다. content 시작점의 Backspace는 내용이 있으면 `<callout>` wrapper만
  제거하고, 내용이 없으면 빈 콜아웃 줄 전체를 삭제한다. leading bar frame chrome의 object replacement는 source
  좌표에서는 제외하고 TextEdit에 돌려줄 때만 장식 offset으로 다시 반영한다.
