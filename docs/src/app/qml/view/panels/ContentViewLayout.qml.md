# `src/app/qml/view/panels/ContentViewLayout.qml`

## Role
`ContentViewLayout.qml` is the content slot surface used by the restored desktop/mobile shell. It exists only to place
the three contents views side by side.

## Composition
- `ContentsView.Gutter`
- `ContentsView.TextEditor`
- `ContentsView.Minimap`

The text editor view is rooted in LVRS `TextEditor` and receives an editor HTML session file path from
`NoteEditorDocumentSession.editorFilePath`.
The note-body wrapper keeps LVRS viewport flicking enabled instead of preferring mobile native text gestures, so a
focused mobile editor can still scroll by finger movement.
It also disables immediate focus-on-press on mobile targets. Mobile cursor placement is driven by LVRS gesture
classification: release-time `pressEnded` taps and deliberate `holdStarted` long presses can focus the editor without
stealing the pointer stream from scrolling.

The gutter receives the same selected source context plus `NoteEditorDocumentSession.parsedLineCount`, the editor
viewport offset, the current logical cursor line index, the sibling editor's fallback logical line height, and a
logical-line placement provider from `TextEditor.qml`. It does not read or parse the note file itself. Its delegate
count follows parsed source lines only; the placement provider merely moves those existing delegates to the rendered
start of each logical line so wrapped continuation rows remain unnumbered.

`gutterVisible` lets route shells suppress the gutter rail without changing the note/session bindings. The mobile
editor route sets it to `false`, matching the existing `minimapVisible: false` compact policy.

The minimap receives the sibling editor's rich-text document, viewport geometry, font tokens, and a view-local scroll
hook so it can render a VSCode-style right-side miniature and viewport thumb without owning parser or persistence
state.

Clipboard resource paste command wiring is limited to the focus-gated `StandardKey.Paste` shortcut dispatch. The layout
asks the `inAppClipboard` context object to capture and import one supported clipboard resource into `.wsresources`,
then passes the returned package metadata to `NoteEditorDocumentSession.insertImportedResourcesIntoSource(...)`. Native
text paste stays inside the LVRS `TextEditor` path when no supported clipboard resource is available.

Editor formatting is handled through the same narrow command shape. Focus-gated shortcuts for `bold`, `italic`,
`underline`, `strikethrough`, `highlight`, and `break` call
`NoteEditorDocumentSession.insertFormatTagIntoSource(...)` with the sibling editor's document, cursor, selection
metadata, and selected visible text, then replace the LVRS document with the C++-projected editor HTML result. Keyboard
shortcuts read the sibling editor's live selection at command time, while the selected-text context menu explicitly opts
into the remembered right-click snapshot. The session maps LVRS RichText plain selection offsets back to visible RAW
source positions before inserting tags, so
existing inline tags before the selection do not shift the selected range. The selected text is read from the live
editor surface with `getText(selectionStart, selectionEnd)` and passed as a repair anchor when a RichText interaction
reports a drifted offset. The C++ session uses the loaded `.wsnbody` RAW source as the format mutation basis, so a
lossy editor RichText projection cannot remove blank source rows before selection is mapped. Applying the same paired
format to the exact text already enclosed by that format toggles it off in `SetTag`; QML does not special-case this.
Highlight is bound to `Meta+Shift+E` /
`Ctrl+Shift+E`, and break is bound to `Meta+Shift+B` / `Ctrl+Shift+B`.

The selected-text format context menu is also owned here. A right-button `TapHandler` on the editor surface opens an
`LV.ContextMenu` only when the sibling editor reports a non-empty selection. Menu entries for `bold`, `italic`,
`underline`, `strikethrough`, and `highlight` reuse the same `applyEditorFormatTag(...)` dispatch path as shortcuts but
pass the snapshot opt-in flag because menu execution happens after the pointer interaction. The layout remembers the live
selection before the context-click begins. The right-button press only guards that
snapshot from being overwritten by native context-click selection drift, and menu item execution uses the remembered
snapshot instead of the shifted live metadata.

## Shell Inputs
`noteEditorSession` is consumed to bind the editor session file into `LV.TextEditor`, to notify the C++ session when
LVRS finishes syncing that session file, and to insert already-imported resource metadata returned from
`inAppClipboard`. Other restored-shell state must not be used to mount parser, projection, renderer, resource editor,
or calendar page logic.

`editorViewModeController` remains removed from this component and must not be reintroduced as a TextEditor backend.

## Guardrails
- Do not add parser, projection, rendering, document snapshot, resource editor, calendar, or editor view-mode wiring here.
- Keep file access limited to the `NoteEditorDocumentSession.editorFilePath -> LV.TextEditor.filePath` binding and
  `syncFinished -> NoteEditorDocumentSession.persistEditorFile(...)` hook.
- Keep gutter wiring limited to selected-note metadata, parsed source line count, fallback editor logical line height,
  logical-line placement provider/revision, current logical cursor line index, viewport offset, and route-level
  visibility.
- Keep minimap wiring limited to editor document text, viewport geometry, source font tokens, and the editor viewport
  scroll hook.
- Keep clipboard paste handling limited to `inAppClipboard` import calls followed by
  `NoteEditorDocumentSession.insertImportedResourcesIntoSource(...)`.
- Keep format shortcut and selected-text context-menu handling limited to command dispatch; static tag allow-lists,
  source mutation, editor HTML projection, and persistence policy stay in `SetTag`, `NoteEditorDocumentSession`, and
  note-body persistence.
- Do not mount `LV.CodeEditor`, raw `TextEdit`, RichText overlays, or legacy editor backend objects.
- Keep the component as a view-only LVRS composition layer.

## Tests
- `test/cpp/suites/qml_contents_view_tests.cpp`
- `test/cpp/suites/qml_inline_format_editor_tests.cpp`

## 한국어

- 이 파일은 restored workspace shell의 content slot surface다.
- 내부 배치는 거터, `LV.TextEditor` wrapper, 미니맵으로 끝난다.
- shell 호환 입력은 받을 수 있고 active note의 editor HTML session file만 `LV.TextEditor.filePath`로 넘긴다.
- 노트 본문 wrapper는 모바일 native text gesture 우선권을 끄고 LVRS viewport flick 경로를 유지하므로, 포커스된
  모바일 에디터에서도 손가락 이동으로 본문을 스크롤할 수 있다.
- 또한 모바일 target에서는 touch begin에서 즉시 포커스하지 않는다. 커서 배치는 LVRS gesture 분류 이후
  release-time `pressEnded` tap과 의도적인 `holdStarted` long press로만 수행되므로, 스크롤 pointer stream을
  빼앗지 않고 소프트웨어 키보드도 touch begin에서 먼저 올라오지 않는다.
- 거터에는 선택 노트 경로, parsed source line count, fallback editor logical line height, logical-line placement
  provider/revision, current logical cursor line index, viewport offset만 전달한다. paragraph wrap으로 생긴 시각
  행은 거터 줄 번호를 늘리지 않으며, continuation row는 번호 없이 비워 둔다. 모바일 editor route는 기존 미니맵
  숨김 정책과 같이 `gutterVisible`을 꺼서 본문 폭을 확보한다.
- 미니맵에는 editor document text, viewport geometry, source font token, editor viewport scroll hook만 전달한다.
- 이미지 paste command wiring은 에디터 포커스 조건을 통과한 `StandardKey.Paste` dispatch에만 둔다.
  `inAppClipboard`가 clipboard 리소스를 `.wsresource` package로 먼저 등록하고, `ContentViewLayout.qml`은 반환
  metadata를 `NoteEditorDocumentSession.insertImportedResourcesIntoSource(...)`로 넘겨 `<resource ...>` 참조만
  삽입한다. 세션이 반환한 editor HTML projection이 리소스 프레임을 표시한다. 지원 리소스가 없으면 일반 텍스트
  paste는 LVRS `TextEditor` native 경로에 맡긴다.
- 포맷 단축키와 선택 텍스트 우클릭 컨텍스트 메뉴는 `NoteEditorDocumentSession.insertFormatTagIntoSource(...)`의
  C++ source/editor HTML 결과를 이어 붙이는 얇은 command wiring으로 제한한다. 하이라이트는 `Cmd+Shift+E`,
  브레이크는 `Cmd+Shift+B`를 기준으로 하고, 비 macOS 변형으로 같은 `Ctrl+Shift+E/B`도 받는다. 단축키는 명령
  시점의 live selection을 사용하고, 컨텍스트 메뉴만 우클릭 interaction 전에 저장한 snapshot을 명시적으로 사용한다.
  실제 포맷 mutation은 C++ 세션이 로드된 `.wsnbody` RAW source를 기준으로 수행하므로, editor RichText projection이
  빈 source row를 손실해도 보이는 selection이 하단 문자열로 밀리지 않아야 한다.
  selected text는 `TextEdit.selectedText` alias에만 의존하지 않고 live editor surface의
  `getText(selectionStart, selectionEnd)`로 읽어 C++ repair anchor로 넘긴다.
  컨텍스트 메뉴는 선택 영역이 있을 때만 열리며 `bold`/`italic`/`underline`/`strikethrough`/`highlight` 항목을
  같은 dispatch로 보낸다. 같은 포맷 wrapper가 정확히 감싼 selection은 QML이 아니라 `SetTag`에서 unwrap toggle로
  처리한다.
- `.wsnbody` parse/serialize는 C++ `NoteEditorDocumentSession`에 맡기며 프로젝션, 렌더링, 캘린더,
  editor view mode 백엔드는 mount하지 않는다.
