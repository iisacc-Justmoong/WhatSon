# `src/app/qml/view/contents/TextEditor.qml`

## Responsibility

`TextEditor.qml` is the only contents-side text editor view. It wraps LVRS `TextEditor` directly.

## Contract

- Root type: `LV.TextEditor`.
- Imports: `QtQuick` and `LVRS 1.0 as LV`.
- `noteBodyFilePath` is the wrapper-owned input for the editor HTML session file prepared by C++.
- `filePath` is bound to `noteBodyFilePath`, letting LVRS perform file read/sync on the session file directly.
- `viewportContentY` relays the LVRS editor viewport scroll offset so the sibling gutter can keep line numbers aligned.
- `editorViewportHeight`, `editorViewportContentHeight`, and `editorViewportWidth` expose the public LVRS editor
  viewport geometry required by the sibling minimap.
- `editorViewportWidth` is also bound into `NoteEditorDocumentSession.editorViewportWidth`, and viewport-width changes
  call the C++ `reprojectResourceFramesForEditorWidth(...)` hook so image resource frames keep a `width:100%` editor
  fill while using an intrinsic media bitmap width that Qt rich text can lay out.
- `editorBottomViewportPaddingRatio` defaults to `0.75`; the wrapper applies that viewport-relative value to the public
  LVRS editor item's bottom padding so the last line can be scrolled into the upper part of the visible editor.
- The wrapper also binds the public LVRS `editorItem.height` to the measured document end plus the same bottom padding.
  This keeps the artificial bottom area in the Flickable content geometry even when the native `bottomPadding` no longer
  expands the viewport by itself.
- The visible artificial bottom area owns a narrow `MouseArea` hit surface. Clicking it focuses the editor and moves the
  cursor to the document end, while `preventStealing: false` leaves drag scrolling available to the LVRS viewport.
- `editorLogicalLineHeight` exposes the LVRS text line-height token used by the sibling gutter as its fallback metric.
- `editorPlainText` reads the public LVRS editor item's plain-text range through `getText(...)`, so the wrapper can
  locate logical editor line starts without depending on RichText HTML markup. CRLF/CR and Qt line/paragraph separator
  characters are normalized to `\n`.
- Resource frames are rendered as one source line and the image-only frame exposes one object-replacement row in the
  editor plain text. The wrapper maps source line starts directly from the public editor plain-text rows.
- `editorCursorLineIndex` is derived from the source-aligned plain-text cursor position, giving the sibling gutter the
  current logical cursor line for its indicator.
- `editorSelectionStart`, `editorSelectionLength`, and `editorSelectedText` expose normalized public LVRS selection
  metadata so the outer content layout can dispatch C++ format commands without installing key handlers inside this
  wrapper. `editorSelectedText` is read from the live editor surface with
  `getText(selectionStart, selectionEnd)` before falling back to `selectedText`, keeping the visible selected text
  available as the C++ RAW-source repair anchor.
- The wrapper attaches `ClipboardEditorPaste` to the public LVRS `editorItem` as the consumable editor paste owner.
  The owner handles only supported image resource paste and returns `true` from its C++ event filter after applying the
  C++-projected editor HTML, preventing the same `Cmd+V` event from reaching native `TextEdit` paste. Unsupported
  clipboard data is left unconsumed so ordinary text paste remains native.
- `scrollEditorViewportTo(contentY)` is a view-local hook used by the minimap to request a viewport scroll without
  introducing an editor backend object.
- `editorLogicalLineMetricFor(lineIndex)` maps a canonical source line index to the rendered rectangle of that line's
  first source-aligned text position through the public LVRS `editorItem.positionToRectangle(...)` API. For resource
  frame lines, the metric height extends to the next source-aligned line when available, keeping later gutter numbers
  below the rendered frame. If parsed source lines outnumber the plain-text rows exposed by LVRS, overflow source lines
  are placed after the last measured editor rectangle instead of reusing the same document-end coordinate. It is only a
  placement helper; it must not drive gutter delegate count.
- `editorPlainTextRevision` bumps when the editor text changes or the wrapper finishes initial discovery.
- `editorLineMetricsRevision` bumps when text or LVRS rendered geometry changes so the sibling gutter can move existing
  source-line delegates while still leaving wrapped continuation rows unnumbered.
- `preferNativeGestures` stays `false` for the WhatSon note body wrapper. The LVRS viewport flick path must remain
  interactive even while the editor has focus, so finger movement on mobile can scroll the note instead of always being
  treated as cursor/text interaction.
- `autoFocusOnPress` stays disabled only on `LV.Theme.mobileTarget`; mobile touch begin no longer focuses the editor.
  The wrapper listens to LVRS `EventListener` gesture classification instead: `pressEnded` focuses only when
  `finalInteractionKind` is `tap`, and `holdStarted` focuses for a deliberate long press before release.
- The wrapper does not install a local `TapHandler`. The original pointer stream remains available to the LVRS viewport
  flick path and native editor surface, while cursor placement is applied from the classified global touch coordinates.
  The only local pointer surface is the bottom-padding `MouseArea`, which exists solely to make the artificial blank
  scroll area clickable and sends the cursor to document end.
- The wrapper uses only public LVRS `TextEditor` surface APIs for editor text, cursor movement, paste owner attachment,
  and paste forwarding.
  It must not reach into the internal `TextDocumentModel` or the removed `editorImeAdapter` object.
- Replacing the current document text for a C++-computed resource or format insertion assigns the C++-projected editor
  HTML to `LV.TextEditor.text`, restores the returned cursor position immediately and once more on the next QML tick,
  then lets LVRS perform its automatic write-through sync.
- Replacing the current document text for a C++ resource-frame viewport reproject follows the same public
  `LV.TextEditor.text`/cursor path and remains a thin view hook; source recovery and frame rendering stay in C++.
- `editorReadOnly` lets the C++ note session freeze the native surface while no note is selected or a note source is
  loading.
- The file does not compute source mutations, resource tags, projection, rendering, persistence, tag management, or
  editor sessions.

## 한국어

- 기준: contents 내부 QML에서 허용되는 세 뷰 중 텍스트 에디터 담당 파일이다.
- 선택된 노트가 있으면 `noteBodyFilePath`를 통해 C++이 만든 editor HTML session file을 편집한다.
- 거터 동기화를 위해 editor viewport의 `contentY`, fallback `editorLogicalLineHeight`,
  `editorLogicalLineMetricFor(lineIndex)`, `editorLineMetricsRevision`을 바깥 layout에 전달한다. 이 metric은
  각 source line의 첫 렌더 위치를 알려 주기 위한 배치 보조값일 뿐, 거터 row 개수를 만들지 않는다.
- 거터가 생성할 줄 번호 개수는 C++ parsed RAW source line count만 사용한다. LVRS editor surface의 visual wrap
  line count나 QML plain-text line count는 거터 row count가 아니다.
- 리소스 프레임은 RAW source에서는 `<resource ... />` 한 줄이며, image-only frame은 editor plain text에서
  object replacement row 하나로 드러난다. `TextEditor.qml`은 별도 chrome row 접기 없이 공개 plain-text row에서
  source-aligned line start를 계산한다.
- parsed source line이 LVRS plain-text row보다 많은 경우에도 남은 source line은 document 끝 좌표 하나를
  재사용하지 않고 마지막 측정 rectangle 아래로 fallback line-height만큼 밀어 배치한다.
- 현재 cursor line indicator를 위해 source-aligned plain-text cursor position으로 계산한 logical
  `editorCursorLineIndex`를 거터에 전달한다. 긴 paragraph가 시각적으로 wrap되어도 같은 logical line number와
  indicator를 유지한다.
- 포맷 단축키 dispatch를 위해 공개 selection 상태를 정규화한 `editorSelectionStart`, `editorSelectionLength`,
  `editorSelectedText`를 바깥 layout에 전달한다. `editorSelectedText`는 우선 live editor surface의
  `getText(selectionStart, selectionEnd)`로 읽어 보이는 선택 문자열과 C++ RAW-source repair anchor가 어긋나지
  않게 한다.
- 미니맵 동기화를 위해 editor viewport의 폭/높이/contentHeight와 `scrollEditorViewportTo(contentY)` hook을
  제공한다.
- 같은 `editorViewportWidth`는 `NoteEditorDocumentSession.editorViewportWidth`에도 전달된다. 폭이 바뀌면 QML은
  현재 editor HTML과 새 폭을 C++ `reprojectResourceFramesForEditorWidth(...)`에 넘기고, C++이 resource frame을
  다시 렌더한 경우에만 공개 `LV.TextEditor.text` 경로로 반영한다.
- 본문 하단에는 viewport 높이의 75%에 해당하는 `bottomPadding`을 공개 LVRS editor item에 적용해 마지막 줄도
  화면 상단 쪽까지 끌어올려 볼 수 있게 한다. 이 인공 여백은 미니맵/스크롤 표면용이며 거터 line-height
  계산에는 참여하지 않는다.
- `bottomPadding`만으로 LVRS viewport content geometry가 늘어나지 않는 경우를 막기 위해, 공개
  `editorItem.height`도 문서 끝 위치와 같은 하단 여백을 합친 높이로 묶는다. 보이는 하단 인공 여백에는
  클릭 전용 `MouseArea`를 두어 클릭 시 커서를 문서 끝으로 보낸다. 이 hit area는 drag steal을 막지 않으므로
  스크롤 제스처는 계속 LVRS viewport가 가져갈 수 있다.
- `preferNativeGestures`는 WhatSon 노트 본문 wrapper에서 `false`로 고정한다. 포커스 중에도 LVRS viewport
  flick 경로가 살아 있어야 모바일 손가락 이동이 항상 커서 조작으로 소비되지 않고 본문 스크롤로 승격된다.
- `autoFocusOnPress`는 `LV.Theme.mobileTarget`에서만 꺼진다. 모바일 에디터는 touch begin 시점에 곧바로
  포커스를 잡지 않는다. 대신 LVRS `EventListener`의 gesture 분류를 사용해 `pressEnded`의
  `finalInteractionKind`가 `tap`일 때만 release 뒤 커서를 배치하고, 의도적인 롱 프레스는 `holdStarted`에서
  별도로 포커스한다.
- 이 wrapper에는 local `TapHandler`를 두지 않는다. 원래 pointer stream은 LVRS viewport flick 경로와 native
  editor surface가 그대로 받을 수 있고, WhatSon은 분류된 global touch 좌표만 커서 위치 계산에 사용한다.
  단, 하단 인공 여백을 클릭 가능한 영역으로 만들기 위한 `MouseArea`는 예외적으로 view-local hit surface로
  유지한다.
- 포맷 command 뒤 C++이 계산한 editor HTML 결과는 공개 `LV.TextEditor.text`/`cursorPosition` API로 반영한다.
  RichText 문서 교체 직후 커서가 초기 위치로 되돌아가지 않도록 즉시 한 번, 다음 QML tick에서 한 번 더 공개
  cursor API로 복원한다. 이미지 resource paste는 `ClipboardEditorPaste` C++ event filter가 공개 editor item에서
  실제 key event를 consume하는 방식으로 소유한다. 지원 리소스가 없는 일반 paste는 공개 `paste()` API와 native
  `TextEdit` 경로에 남긴다.
- 내부 `TextDocumentModel`이나 제거된 `editorImeAdapter` objectName에는 의존하지 않는다.
- `.wsnbody` XML 컨테이너 자체를 이 파일에 직접 연결하지 않는다.
- `LV.CodeEditor`, raw `TextEdit`, RichText overlay, parser/projection/rendering bridge를 추가하지 않는다.
