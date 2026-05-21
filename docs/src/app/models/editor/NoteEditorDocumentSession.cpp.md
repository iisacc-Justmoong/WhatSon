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
   only applies the pull when the filesystem copy is newer and no local editor mutation has made the session source
   authoritative for that note.
6. QML binds that session file into LVRS `TextEditor.filePath`, keeps the parsed source line count as session metadata,
   binds the current editor viewport width into the session, and uses the parsed line count as the gutter delegate
   count. The sibling editor supplies only rendered start positions for those parsed source lines.
7. When LVRS emits `syncFinished(path)`, QML sends the editor document text to
   `requestEditorIdleRawPush(...)`; when the editor surface modified count increases, QML sends the same surface
   payload to `requestEditorModifiedCountRawPush(...)`. Both routes pass through
   `WhatSonEditorRawPushController`, then the session converts the editor document HTML back into canonical source text
   and delegates persistence through `ContentsNoteManagementCoordinator` so `.wsnbody` is reserialized and
   `parsedLineCount` is refreshed.
   Modified-count pushes update the active session source immediately, before any delayed persistence callback can run.
   Idle pushes then persist that active session source instead of trusting an older sync-finished payload, so the last
   typed character or last inserted source component is not lost when a stale editor-file sync arrives late.
   If that persistence writes a timestamped `.wsnversion` diff, the coordinator signal is forwarded as
   `hubFilesystemMutated()` for the composition-root hub-sync wiring.
   Before note context changes or clears, the session asks the same push controller to flush the active surface. If that
   note-departure flush has no live payload, or if it carries an older pending payload, the session still persists the
   locally authoritative active source instead of rereading a stale mounted session file.
8. Editor format shortcuts call `insertFormatTagIntoSource(...)`; the session mutates the loaded `.wsnbody` RAW source,
   maps the rendered selection to RAW visible-character positions, applies `SetTag`, returns a fresh editor HTML
   projection, and maps the source cursor back to the rendered editor cursor position.
9. Clipboard resource paste calls `insertImportedResourcesIntoSource(...)` only after `InAppClipboardManager` has persisted
   the resource package. The session inserts RAW resource tags, commits the active-note source through the
   note-management queue before returning a successful paste result, discards any pre-paste pending push for the same
   session file, writes the matching editor session file, and returns an editor HTML projection that renders each
   standalone resource source line as a resource frame. If a collapsed paste cursor resolves to the start of the line
   after an empty source line, that empty line is reused for the resource tag instead of creating another line below it.
10. Editor key filters call `handleCalloutBoundaryKeyInSource(...)` before native text handling for plain
    Backspace/Enter on callout boundaries. The session maps the rendered cursor back to loaded RAW source, delegates the
    callout-specific boundary rule to `component/Callout`, applies the returned source edit, and reprojects the editor
    document. The callout frame-chrome object replacement and renderer-only line characters are skipped for source
    mapping and then re-applied when returning the decorated TextEdit cursor position.

## Guardrails

- The session creates blank/editor session files outside the note package.
- The session keeps a file-path-to-note-context map so late sync signals from a previous editor file still persist to
  the note that originally owned that file.
- Note entry/open RAW pulls are routed through `file/sync/WhatSonEditorRawPullController`; the session remains the
  load callback and editor projection owner.
- The loaded note's `lastModified` timestamp is kept with the editor session-file context. Later RAW pushes pass that
  base timestamp into the note-management queue so timestamp conflict resolution can tell whether the filesystem
  changed after this editor pull.
- Active-note idle RAW pulls reuse that same timestamp context in the opposite direction: a filesystem pull whose
  `lastModified` is not strictly newer is ignored and reported through `editorFilesystemPullIgnored(...)`. If the note
  currently has a locally authoritative editor session source, even a newer filesystem body is treated as an external
  snapshot and is ignored with `session-authoritative`; the session reprojects and repushes the active source instead
  of replacing the live editor with stale text. A freshly loaded note whose session source has not yet been changed
  locally can still be replaced by a newer filesystem body, in which case the session file is rewritten,
  `editorDocumentTextPulled(...)` is emitted, and the stored base timestamp is updated.
- Idle, modified-count, and note-departure RAW pushes are routed through
  `file/sync/WhatSonEditorRawPushController`; the session remains the conversion/write callback owner.
- The active editor session source becomes the save/sync truth once local editor mutation refreshes it. Modified-count
  pushes, resource paste, format-tag insertion, callout boundary edits, accepted idle pushes, and frame reprojection can
  all refresh that source. Later idle pushes, note-departure flushes, and late persistence-finished callbacks must not
  rewind it to an older payload. `persistEditorFile(...)` is the explicit file-based fallback and therefore reads the
  current mounted editor session file, allowing real user deletions such as a removed resource object to become the next
  active source. Internal note-departure fallback uses `persistEditorFileForRawPush(...)` so stale session-file contents
  cannot erase a just-inserted resource before the note is reopened.
- Re-selecting the same note keeps the existing session file intact, so unsaved editor state is not overwritten
  by a redundant body reload.
- The open-count update is a selected-note bind side effect owned by `ContentsNoteManagementCoordinator`; editor QML must
  not write statistic fields directly.
- The session computes parsed RAW source line count in C++ so QML gutter code does not read or parse note files. The
  gutter uses that metadata as its row count and must not derive row count from LVRS rendered wrap-line geometry.
- Imported-resource insertion consumes metadata returned by `InAppClipboardManager`; it must not inspect MIME data or create
  resource packages itself.
- Imported-resource insertion is a persistence boundary as well as a projection boundary: once the resource package
  exists and the active note is known, the canonical RAW `<resource ... />` line is saved to `.wsnbody` before the paste
  result is reported, and the matching resource-frame HTML is written to the mounted `.wsnsource` at the same boundary.
  The same boundary discards any older pending raw push for that session file so a pre-paste modified-count snapshot
  cannot overwrite the newly inserted resource line after the paste command returns.
- Standalone `<resource ... />` source lines are atomic editor slots. The session renders them with
  `component/ResourceImageFrame` and wraps that frame in `whatson-resource-source` markers so the persistence boundary can
  recover the exact canonical source tag. Image resources whose package asset resolves from the active note/hub context
  render as `<img src="file://...">` inside the Figma `292:50` resource frame; unresolved or non-image resources still
  render as a visible resource frame with the stored resource reference.
- Resource frame projection uses `editorViewportWidth` from QML as the media raster's intrinsic width, while preserving
  a structured `width="100%"` frame, `height:auto` media, and dynamic centered image placement in the emitted HTML.
  The first projection records `data-frame-display-height`; `reprojectResourceFramesForEditorWidth(...)` parses that
  existing marker and reuses it as the locked frame height, so a window resize changes frame width and x-axis centering
  without changing the initial auto height. The same reproject hook also recognizes callout frames and regenerates their
  leading-bar image from the currently edited content. QML schedules that hook through a short debounce after text
  changes, so wrapped callout chrome updates without waiting for idle persistence and without replacing the native
  document on every keystroke.
- When Qt serializes the rich editor document and strips those HTML markers, image frames remain as rich-text object
  replacement characters. `persistEditorFile(...)` restores those object placeholders from the active canonical source
  resource lines before delegating `.wsnbody` persistence, preserving the resource reference across real editor save
  round-trips. The debounced `reprojectResourceFramesForEditorWidth(...)` path applies the same active-source
  restoration when the live editor HTML still contains a resource frame but the `whatson-resource-source` comments were
  stripped, so a freshly pasted image frame is not interpreted as a deleted resource during immediate viewport
  re-projection. If a markerless live frame also loses its rich-text object placeholder, idle RAW push compares the
  source text with resource source lines removed and keeps the active canonical resource source instead of persisting the
  renderer's empty frame blocks as new `<paragraph></paragraph>` rows.
- For notes that already contain amplified empty paragraphs around a resource frame, Backspace/Delete edits are allowed
  to reduce those empty source lines. The restore path tracks the active blank-line count around each resource line and
  subtracts the live frame's serialized padding before preserving blanks, so a one-line deletion cannot re-save as a
  larger run of empty paragraphs.
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
  mapped. Custom `<style ...>` wrappers are invisible source chrome while their child text remains selectable. If the
  RichText selection offset has drifted, the session compares that selected text with the visible source span and repairs
  the source range before calling `SetTag`.
- Callout boundary keys are source mutations applied by this class, but the callout-specific range parsing and edit
  planning live in `component/Callout`. Backspace at the callout content start removes only the `<callout>` wrapper when
  content exists, or removes the whole empty callout source line when it does not. Enter/Return inside a callout never
  inserts text inside the wrapper; it places the cursor on the following source line outside the wrapper, creating that
  following line for a trailing callout. Because the visual callout frame uses one generated chrome object for the
  leading bar, the session supplies source-visible cursor mapping and applies the `CalloutBoundaryEdit` returned by the
  component. If Enter is pressed on the frame chrome immediately before the callout content, `component/Callout` plans a
  new line before `<callout>` instead of consuming the key as an inside-callout exit.
- It must not expose the raw XML body file as the editor file path.

## 한국어

- 이 구현은 `.wsnbody` XML을 그대로 에디터에 띄우지 않는다.
- 선택된 노트의 본문을 RAW source로 파싱한 뒤 LVRS `TextEditor`가 줄바꿈을 보존해 렌더할 수 있는 editor HTML
  session file로 투영하고, LVRS 저장 이벤트는 다시 canonical source로 복원해 `.wsnbody` 직렬화 경로로 연결한다.
- 활성 노트의 editor session source는 로컬 editor mutation으로 갱신된 뒤 저장과 sync의 권위가 된다. editor surface
  revision이 증가하면 세션은 지연 저장 전에 canonical source를 먼저 갱신하고, 이후 늦게 도착한 idle sync payload나
  persistence 완료 콜백이 그 source를 과거 스냅샷으로 되감지 못하게 한다. 이 계약은 이미지 리소스뿐 아니라 일반
  텍스트의 마지막 글자에도 적용된다.
  단, `persistEditorFile(...)`은 명시적인 파일 기반 fallback이므로 현재 mounted editor session file을 다시 읽어
  실제 삭제된 resource object 같은 사용자 삭제를 새 active source로 반영한다.
  노트 이탈 flush는 내부 `persistEditorFileForRawPush(...)` 경로를 사용해, live payload가 없거나 pending payload가
  낡았더라도 방금 삽입한 resource source를 mounted session file의 낡은 내용으로 덮어쓰지 않는다.
- parsed RAW source line count는 이 C++ 세션이 계산한다. 거터 표시 row 개수는 이 metadata만 따른다.
  paragraph wrap으로 생긴 rendered row count는 거터 row count에 참여하지 않는다.
- QML은 공개 LVRS editor item 폭을 `editorViewportWidth`로 전달한다. 이 값은 resource frame media raster의
  intrinsic width가 되어 Qt rich text에서 프레임이 editor 폭을 채우도록 만들고, 실제 이미지 표시 박스는 그 frame
  폭 안에서 다시 중앙 정렬된다.
- clipboard resource paste는 `InAppClipboardManager`가 `.wsresource` package를 먼저 만든 뒤 이 세션의
  `insertImportedResourcesIntoSource(...)`로 들어온다. 세션은 본문 RAW source에 `<resource ... />` 참조를
  삽입하고, 활성 노트라면 성공 결과를 반환하기 전에 note-management queue를 통해 `.wsnbody`까지 저장한 뒤
  같은 source에서 만든 editor HTML projection을 mounted `.wsnsource` session file에도 쓴다. 이 즉시 저장 경계
  때문에 붙여넣기 직후 재시작/동기화 시 `<resource ... />` 참조가 사라지지 않는다.
  collapsed cursor가 빈 source line 바로 다음 줄 시작으로 들어와도 세션은 그 빈 줄을 resource line으로
  재사용하므로, 이미지가 현재 커서 줄 아래에 새 빈 공간을 만든 뒤 붙지 않는다.
  standalone resource source line은 editor HTML에서
  `component/ResourceImageFrame`을 통해 Figma `292:50` 형태의 `whatson-resource-frame`으로 렌더링되고, 이미지 package
  asset을 active note/hub 기준으로 찾을 수 있으면 `file://` 이미지로 표시된다. 저장 시에는
  `whatson-resource-source` marker가 다시 canonical `<resource ... />` source tag로 복구된다. Qt RichText 직렬화가
  marker를 제거한 경우에도 `persistEditorFile(...)`은 active canonical source의 resource line과 이미지 object
  placeholder를 기준으로 `<resource ... />`를 복원한다. 즉시 viewport 재투영도 live resource frame이 남아 있으면
  같은 active source 복원 경로를 사용하므로, paste 직후 marker comment만 사라진 프레임을 삭제로 오해하지 않는다.
  markerless live frame이 rich-text object placeholder까지 잃은 경우에도 idle RAW push는 resource line을 제거한
  주변 source가 같은지 확인한 뒤 active canonical source를 유지한다. 따라서 renderer가 만든 빈 frame block이
  시간차 저장 때마다 `<paragraph></paragraph>`로 누적되지 않는다.
  이미 빈 paragraph가 증폭된 노트에서도 Backspace/Delete가 줄인 빈 source line은 유지된다. 세션은 resource 주변의
  active blank-line 개수를 추적하고 live frame 직렬화 패딩을 뺀 뒤 빈 줄을 보존하므로, 지운 빈 paragraph가 저장
  경계에서 더 큰 빈 줄 묶음으로 되살아나지 않는다.
  Backspace/Delete 뒤 이미지 object가 사라진 경우에는
  resource frame이 삭제된 것으로 본다. persistence 성공 콜백 뒤에는 `.wsnbody` serializer/read-back 경계의
  canonical source로 parsed line count를 다시 맞춰, 삭제된 atomic frame 뒤의 임시 trailing line이 거터 계약에
  남지 않게 한다.
- editor viewport 폭이 바뀌거나 callout 텍스트가 편집되면 QML이 짧은 debounce 뒤
  `reprojectResourceFramesForEditorWidth(...)`를 호출한다. 이 함수는 현재 editor HTML을 source로 복원하고, resource
  frame 또는 callout frame chrome이 있는 경우 새 폭으로 projection을 다시 만든다. resource는 기존
  `data-frame-display-height`를 읽어 frame height를 초기 auto 값으로 고정하므로, resize는 frame 폭과 x축 중앙
  offset만 바꾸고 높이는 바꾸지 않는다. callout은 현재 편집된 content의 wrap 높이에 맞춰 좌측 막대 이미지를 다시
  생성한다.
- 포맷 단축키는 이 세션의 `insertFormatTagIntoSource(...)`로 들어오며, 로드된 `.wsnbody` RAW source를 기준으로
  mutation한다. editor RichText projection이 빈 source row를 손실해도 selection 좌표는 `.wsnbody` source의 논리
  row를 기준으로 변환한다. `<next />`와 `<br>` 같은 source-level rendered break는 selection 논리 좌표에서 newline
  1글자로 센다. 좌표가 selected text와 맞지 않으면 실제 visible source에서 selected text 위치를 다시 찾아 paragraph
  밖으로 wrapper가 새는 것을 막는다.
  같은 태그가 정확히 감싼 selection이면 `SetTag`가 wrapper를 제거하는 toggle 결과를 반환한다.
  첫 렌더링 위치로 매핑한다.
- 콜아웃 경계 키는 이 세션이 적용하지만, 콜아웃 source range 탐색과 boundary edit 계획은 `component/Callout`이
  맡는다. content 시작점의 Backspace는 내용이 있으면 `<callout>` wrapper만 제거하고, 내용이 없으면 빈 콜아웃 줄
  전체를 삭제한다. 콜아웃 내부 Enter/Return은 wrapper 안에 줄바꿈을 넣지 않고 닫는 태그 뒤 다음 source line으로
  커서를 옮긴다. trailing callout이면 그 다음 줄을 새로 만든다. 세션은 source-visible cursor mapper를 제공하고
  `CalloutBoundaryEdit`를 active source와 editor HTML projection에 반영한다. content 바로 왼쪽의 frame chrome
  위치에서 Enter를 누르면 `component/Callout`이 `<callout>` 앞 source 위치에 새 줄 삽입을 계획해, 거터 라인이
  줄지 않고 정상적으로 늘어나게 한다.
