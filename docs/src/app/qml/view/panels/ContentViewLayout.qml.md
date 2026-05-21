# `src/app/qml/view/panels/ContentViewLayout.qml`

## Role
`ContentViewLayout.qml` is the content slot surface used by the restored desktop/mobile shell. It switches between the
note editor contents views, the image-resource viewer, and the navigation-driven calendar overlays.

## Composition
- `ContentsView.Gutter`
- `ContentsView.ImageEditor`
- `ContentsView.TextEditor`
- `ContentsView.Minimap`
- `CalendarView.DayCalendarPage`
- `CalendarView.WeekCalendarPage`
- `CalendarView.MonthCalendarPage`
- `CalendarView.YearCalendarPage`

The image editor is mounted only when the active list model exposes `currentResourceEntry` and the selected entry is an
image resource. Its input is the current resource entry map; it does not create a generic resource editor surface.

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

Calendar overlays are mounted in a separate `CalendarView` stack above the editor/image row. When any calendar overlay
is active, the editor/image row is hidden and disabled so pointer and focus ownership moves to the visible calendar
page. The overlay receives the day/week/month/year controllers from `Main.qml` via `BodyLayout.qml`. Day, week, and
month note chips call `LibraryHierarchyController.activateNoteById(...)`, switch the sidebar back to the Library
hierarchy, and dismiss the visible overlay after a successful activation. Year view drill-down sets the month
controller cursor before requesting the month overlay so the selected month/date is preserved.

Clipboard resource paste is owned by the editor surface, not by an observer shortcut path. The layout passes
`EditorInputCommandFilter`, `ClipboardEditorPaste`, `InAppClipboardManager`, and `NoteEditorDocumentSession` into
`ContentsView.TextEditor`; the wrapper installs the input filter on the public LVRS editor item. The layout does not
install `StandardKey.Paste`, does not listen to `LV.RuntimeEvents.keyPressed`, and does not use a clock-based dedup
window. When the C++ filter handles a supported image resource it consumes the underlying key event, so native
`TextEdit` paste cannot run after the resource paste. Unsupported clipboard content remains unconsumed and continues
through the LVRS native paste path.

Editor formatting is handled through the same narrow command shape. Focus-gated shortcuts for `bold`, `italic`,
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
`Ctrl+Shift+E`, break is bound to `Meta+Shift+B` / `Ctrl+Shift+B`, callout is bound to `Meta+Shift+C` /
format shortcuts: a selection becomes the rendered callout contents, while an empty selection inserts an empty visual
nested task item, rather than a standalone task wrapper.

renderer draws the visible checkbox fallback inside the task row so the shape is present before or without overlay
refresh. The layout asks
inside the LVRS `TextEditor` component tree. This keeps the interactive checkbox control out of the editor's clipped
Flickable/TextEdit internals while preserving editor text selection and caret ownership.
The overlay derives its hit target from the editable task body position plus `checkboxSize`, `checkboxTextGap`, and
`checkboxRadius`, so the native control keeps the Figma 17px rounded-square shape instead of collapsing to an
empty-text implicit size without requiring private marker characters in the editor text.
The overlay is cached and refreshed through a short timer after document or line-metric changes; it is not computed as
a synchronous rich-text binding while a note is opening. These checkboxes also opt out of tab and item focus, so they
can be clicked without stealing the LVRS editor caret or hiding the text cursor during initial note load.
chrome remains an interactive completion control rather than a text input region.

The selected-text format context menu is also owned here. A right-button `TapHandler` on the editor surface opens an
`LV.ContextMenu` only when the sibling editor reports a non-empty selection. Menu entries for `bold`, `italic`,
`underline`, `strikethrough`, and `highlight` reuse the same `applyEditorFormatTag(...)` dispatch path as shortcuts but
pass the snapshot opt-in flag because menu execution happens after the pointer interaction. The layout remembers the live
selection before the context-click begins. The right-button press only guards that
snapshot from being overwritten by native context-click selection drift, and menu item execution uses the remembered
snapshot instead of the shifted live metadata.

## Shell Inputs
`noteListModel` may expose `currentResourceEntry` for the resources hierarchy list. `ContentViewLayout.qml` reads that
entry to decide whether the visible content surface is `ContentsView.ImageEditor` or the note editor surface. Image
viewer routing is based on the entry type/format and the already-resolved `source`, `resolvedPath`, or `resourcePath`
fields. The layout does not parse resource packages or mutate resource metadata.

`noteEditorSession` is consumed to bind the editor session file into `LV.TextEditor`, to notify the C++ session when
LVRS finishes reading and syncing the currently mounted session file, to forward editor modified-count increments with
the mounted path, revision, and live document text for RAW push scheduling, and to insert already-imported resource
metadata returned from `inAppClipboard`. `readFinished(path)` must match the current `editorSourceFilePath` before the
session is marked ready for RAW push; `syncFinished(path)` is ignored when `path` is not the current
`editorSourceFilePath`, so an old session-file notification cannot pair that old path with the current editor payload.
Other restored-shell state must not be used to mount parser, projection, renderer, generic resource editor, or editor
view-mode backend logic. Calendar routing is limited to the already-owned day/week/month/year controllers and overlay
visibility signals.
When the session emits `editorDocumentTextPulled(...)` for a newer idle filesystem pull, the layout replaces the LVRS
document text through the sibling editor's public wrapper hook and suppresses the resulting revision tick from being
classified as a local modified-count push.

`editorViewModeController` remains removed from this component and must not be reintroduced as a TextEditor backend.

## Guardrails
- Do not add parser, projection, document snapshot, generic resource editor, or editor view-mode wiring here.
- Keep calendar wiring limited to the four overlay pages, controller pass-through, year-to-month drill-down, and
  note-chip activation bridge.
- Keep image resource routing limited to `ResourcesListModel.currentResourceEntry` and `ContentsView.ImageEditor`.
- Keep file access limited to the `NoteEditorDocumentSession.editorFilePath -> LV.TextEditor.filePath` binding and
  the thin RAW-push trigger calls into `NoteEditorDocumentSession`.
- Keep gutter wiring limited to selected-note metadata, parsed source line count, fallback editor logical line height,
  logical-line placement provider/revision, current logical cursor line index, viewport offset, and route-level
  visibility.
- Keep minimap wiring limited to editor document text, viewport geometry, source font tokens, and the editor viewport
  scroll hook.
- Keep editor native key interception limited to `EditorInputCommandFilter`; resource import, source insertion, and
  resource reload stay in C++.
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
- 내부 배치는 노트 선택 시 거터, `LV.TextEditor` wrapper, 미니맵으로 끝나며, 이미지 리소스 선택 시
  `ImageEditor.qml`만 콘텐츠 표면에 표시한다. 네비게이션 캘린더 버튼이 열린 상태에서는 같은 content slot 위에
  day/week/month/year calendar overlay를 표시하고 editor/image row를 비활성화한다.
- shell 호환 입력은 받을 수 있고 active note의 editor HTML session file만 `LV.TextEditor.filePath`로 넘긴다.
- 리소스 하이어라키 list model이 `currentResourceEntry`를 제공하면, type/format과 `source`/`resolvedPath`/
  `resourcePath`를 기준으로 이미지 리소스 선택 여부를 판단한다. 이 판단은 viewer 전환만 수행하며 `.wsresource`
  package 파싱이나 resource metadata 저장은 하지 않는다.
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
- 이미지 paste command wiring은 observer shortcut이 아니라 editor surface의 C++ input filter가 가진다.
  `ContentViewLayout.qml`은 `EditorInputCommandFilter`, `ClipboardEditorPaste`, `InAppClipboardManager`,
  `NoteEditorDocumentSession`을 `ContentsView.TextEditor`에 넘기기만 한다. `StandardKey.Paste`, LVRS
  `RuntimeEvents.keyPressed`, 시간 기반 dedup window는 사용하지 않는다. 지원 이미지 리소스가 있으면 C++ filter가
  `.wsresource` package 등록, `<resource ...>` 삽입, editor HTML projection 반영을 위임한 뒤 실제 key event를
  consume한다. 지원 리소스가 없으면 이벤트를 consume하지 않아 일반 텍스트 paste는 LVRS native 경로로 계속 흐른다.
- 포맷 단축키와 선택 텍스트 우클릭 컨텍스트 메뉴는 `NoteEditorDocumentSession.insertFormatTagIntoSource(...)`의
  C++ source/editor HTML 결과를 이어 붙이는 얇은 command wiring으로 제한한다. 하이라이트는 `Cmd+Shift+E`,
  브레이크는 `Cmd+Shift+B`, 콜아웃은 `Cmd+Shift+C`를 기준으로 하고, 비 macOS 변형으로 같은
  `Ctrl+Shift+E/B/C`도 받는다. 단축키는 명령
  시점의 live selection을 사용하고, 컨텍스트 메뉴만 우클릭 interaction 전에 저장한 snapshot을 명시적으로 사용한다.
  실제 포맷 mutation은 C++ 세션이 로드된 `.wsnbody` RAW source를 기준으로 수행하므로, editor RichText projection이
  빈 source row를 손실해도 보이는 selection이 하단 문자열로 밀리지 않아야 한다.
  selected text는 `TextEdit.selectedText` alias에만 의존하지 않고 live editor surface의
  `getText(selectionStart, selectionEnd)`로 읽어 C++ repair anchor로 넘긴다.
  컨텍스트 메뉴는 선택 영역이 있을 때만 열리며 `bold`/`italic`/`underline`/`strikethrough`/`highlight` 항목을
  같은 dispatch로 보낸다. 같은 포맷 wrapper가 정확히 감싼 selection은 QML이 아니라 `SetTag`에서 unwrap toggle로
  처리한다. 콜아웃 단축키는 selection이 있으면 해당 텍스트를 콜아웃 내부 콘텐츠로 쓰고, selection이 없으면 빈
  콜아웃 시각 프레임을 즉시 만든다.
- `.wsnbody` parse/serialize는 C++ `NoteEditorDocumentSession`에 맡기며 프로젝션, 렌더링, generic resource editor,
  editor view mode 백엔드는 mount하지 않는다. Calendar overlay는 이미 노출된 calendar controller와
  `LibraryHierarchyController.activateNoteById(...)` bridge만 사용한다.
- LVRS session file sync와 editor revision 증가 이벤트는 QML에서 직접 저장하지 않고
  `NoteEditorDocumentSession`의 RAW push 요청 함수로 넘긴다. 실제 debounce, note-departure flush, RAW 변환은
  C++ 쪽 책임이다.
- C++ 세션이 idle filesystem pull에서 더 최신 본문을 찾으면 `editorDocumentTextPulled(...)`를 내보낸다. 이
  layout은 공개 editor wrapper hook으로 문서를 교체하되, 그 교체로 발생한 revision tick은 로컬 수정 push로
  분류하지 않는다.
