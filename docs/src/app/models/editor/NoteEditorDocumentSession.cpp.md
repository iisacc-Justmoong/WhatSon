# `src/app/models/editor/NoteEditorDocumentSession.cpp`

## Responsibility

Implements the active note editor document session.

## Runtime Flow

1. Active-note changes are read from `NoteActiveStateTracker`.
2. `ContentsNoteManagementCoordinator` loads the selected note body as canonical editor-facing RAW source.
3. The source is projected into editor HTML and written to a cache/session `.wsnsource` file so LVRS
   `TextEditor` receives explicit rich-text line breaks.
4. QML binds that session file into LVRS `TextEditor.filePath`, keeps the parsed source line count as session metadata,
   binds the current editor viewport width into the session, and uses the parsed line count as the gutter delegate
   count. The sibling editor supplies only rendered start positions for those parsed source lines.
5. When LVRS emits `syncFinished(path)`, QML calls `persistEditorFile(path)`, the session converts the editor document
   HTML back into canonical source text, and then delegates persistence through `ContentsNoteManagementCoordinator`
   so `.wsnbody` is reserialized and `parsedLineCount` is refreshed.
6. Editor format shortcuts call `insertFormatTagIntoSource(...)`; the session mutates the loaded `.wsnbody` RAW source,
   maps the rendered selection to RAW visible-character positions, applies `SetTag`, returns a fresh editor HTML
   projection, and maps the source cursor back to the rendered editor cursor position.
7. Clipboard resource paste calls `insertImportedResourcesIntoSource(...)` only after `InAppClipboardManager` has persisted
   the resource package. The session inserts RAW resource tags and returns an editor HTML projection that renders each
   standalone resource source line as a resource frame.

## Guardrails

- The session creates blank/editor session files outside the note package.
- The session keeps a file-path-to-note-context map so late sync signals from a previous editor file still persist to
  the note that originally owned that file.
- Re-selecting the same note keeps the existing session file intact, so unsaved editor state is not overwritten
  by a redundant body reload.
- The session computes parsed RAW source line count in C++ so QML gutter code does not read or parse note files. The
  gutter uses that metadata as its row count and must not derive row count from LVRS rendered wrap-line geometry.
- Imported-resource insertion consumes metadata returned by `InAppClipboardManager`; it must not inspect MIME data or create
  resource packages itself.
- Standalone `<resource ... />` source lines are atomic editor slots. The session renders them with
  `component/ResourceImageFrame` and wraps that frame in `whatson-resource-source` markers so the persistence boundary can
  recover the exact canonical source tag. Image resources whose package asset resolves from the active note/hub context
  render as `<img src="file://...">` inside the Figma `292:50` resource frame; unresolved or non-image resources still
  render as a visible resource frame with the stored resource reference.
- Resource frame projection uses `editorViewportWidth` from QML as the preview bitmap's intrinsic width, while preserving
  `width="100%"` and `height:auto` in the emitted HTML. `reprojectResourceFramesForEditorWidth(...)` can re-render the
  current editor document when the editor viewport width changes.
- When Qt serializes the rich editor document and strips those HTML markers, image frames remain as rich-text object
  replacement characters. `persistEditorFile(...)` restores those object placeholders from the active canonical source
  resource lines before delegating `.wsnbody` persistence, preserving the resource reference across real editor save
  round-trips.
- If Backspace/Delete removes the rich-text object but leaves resource-frame chrome text such as the type label, menu
  dots, or file name, `persistEditorFile(...)` treats that chrome as the residue of one deleted resource component and
  removes it instead of saving it as ordinary note text.
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
- QML은 공개 LVRS editor item 폭을 `editorViewportWidth`로 전달한다. 이 값은 resource frame preview bitmap의
  intrinsic width가 되어 Qt rich text에서 프레임이 editor 폭을 채우도록 만든다.
- clipboard resource paste는 `InAppClipboardManager`가 `.wsresource` package를 먼저 만든 뒤 이 세션의
  `insertImportedResourcesIntoSource(...)`로 들어온다. 세션은 본문 RAW source에 `<resource ... />` 참조를
  삽입하고 editor HTML projection을 반환한다. standalone resource source line은 editor HTML에서
  `component/ResourceImageFrame`을 통해 Figma `292:50` 형태의 `whatson-resource-frame`으로 렌더링되고, 이미지 package
  asset을 active note/hub 기준으로 찾을 수 있으면 `file://` 이미지로 표시된다. 저장 시에는
  `whatson-resource-source` marker가 다시 canonical `<resource ... />` source tag로 복구된다. Qt RichText 직렬화가
  marker를 제거한 경우에도 `persistEditorFile(...)`은 active canonical source의 resource line과
  `ResourceFrame::renderedTextLines(...)`를 기준으로 이미지 object placeholder를 다시 `<resource ... />`로 복원한다.
  반대로 Backspace/Delete 뒤 이미지 object는 사라지고 frame chrome 텍스트만 남은 경우에는 그 chrome을 일반 본문
  텍스트로 저장하지 않고 삭제된 resource component의 잔여물로 제거한다.
- editor viewport 폭이 바뀌면 `reprojectResourceFramesForEditorWidth(...)`가 현재 editor HTML을 source로 복원하고,
  resource frame이 있는 경우에만 새 폭으로 frame preview를 다시 만든다.
- 포맷 단축키는 이 세션의 `insertFormatTagIntoSource(...)`로 들어오며, 로드된 `.wsnbody` RAW source를 기준으로
  mutation한다. editor RichText projection이 빈 source row를 손실해도 selection 좌표는 `.wsnbody` source의 논리
  row를 기준으로 변환한다. `<next />`와 `<br>` 같은 source-level rendered break는 selection 논리 좌표에서 newline
  1글자로 센다. 좌표가 selected text와 맞지 않으면 실제 visible source에서 selected text 위치를 다시 찾아 paragraph
  밖으로 wrapper가 새는 것을 막는다.
  같은 태그가 정확히 감싼 selection이면 `SetTag`가 wrapper를 제거하는 toggle 결과를 반환한다.
