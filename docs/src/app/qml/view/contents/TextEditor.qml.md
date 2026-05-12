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
- `editorBottomViewportPaddingRatio` defaults to `0.5`; the wrapper applies that viewport-relative value to the public
  LVRS editor item's bottom padding so the last line can be scrolled up near the middle of the visible editor.
- `editorLogicalLineHeight` exposes the LVRS text line-height token used by the sibling gutter as its fallback metric.
- `editorPlainText` reads the public LVRS editor item's plain-text range through `getText(...)`, so the wrapper can
  locate logical editor line starts without depending on RichText HTML markup. CRLF/CR and Qt line/paragraph separator
  characters are normalized to `\n`.
- `editorCursorLineIndex` is derived from the plain-text cursor position, giving the sibling gutter the current logical
  cursor line for its indicator.
- `editorSelectionStart` and `editorSelectionLength` expose normalized public LVRS selection metadata so the outer
  content layout can dispatch C++ format commands without installing key handlers inside this wrapper.
- `scrollEditorViewportTo(contentY)` is a view-local hook used by the minimap to request a viewport scroll without
  introducing an editor backend object.
- `editorLogicalLineMetricFor(lineIndex)` maps a canonical source line index to the rendered rectangle of that line's
  first text position through the public LVRS `editorItem.positionToRectangle(...)` API. It is only a placement helper;
  it must not drive gutter delegate count.
- `editorPlainTextRevision` bumps when the editor text changes or the wrapper finishes initial discovery.
- `editorLineMetricsRevision` bumps when text or LVRS rendered geometry changes so the sibling gutter can move existing
  source-line delegates while still leaving wrapped continuation rows unnumbered.
- `preferNativeGestures` follows `LV.Theme.mobileTarget`. Desktop must keep LVRS wheel scrolling enabled even while
  the editor has focus; forcing mobile native gestures globally makes note body scrolling appear disabled.
- The wrapper uses only public LVRS `TextEditor` surface APIs for editor text, cursor movement, and paste forwarding.
  It must not reach into the internal `TextDocumentModel` or the removed `editorImeAdapter` object.
- Replacing the current document text for a C++-computed resource or format insertion assigns the C++-projected editor
  HTML to `LV.TextEditor.text`, restores the returned cursor position immediately and once more on the next QML tick,
  then lets LVRS perform its automatic write-through sync.
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
- 현재 cursor line indicator를 위해 plain-text cursor position으로 계산한 logical `editorCursorLineIndex`를
  거터에 전달한다. 긴 paragraph가 시각적으로 wrap되어도 같은 logical line number와 indicator를 유지한다.
- 포맷 단축키 dispatch를 위해 공개 selection 상태를 정규화한 `editorSelectionStart`와 `editorSelectionLength`를
  바깥 layout에 전달한다.
- 미니맵 동기화를 위해 editor viewport의 폭/높이/contentHeight와 `scrollEditorViewportTo(contentY)` hook을
  제공한다.
- 본문 하단에는 viewport 높이의 50%에 해당하는 `bottomPadding`을 공개 LVRS editor item에 적용해 마지막 줄도
  화면 중앙 부근까지 끌어올려 볼 수 있게 한다. 이 인공 여백은 미니맵/스크롤 표면용이며 거터 line-height
  계산에는 참여하지 않는다.
- `preferNativeGestures`는 `LV.Theme.mobileTarget`을 따른다. 데스크톱에서 이를 강제로 켜면 포커스 중
  LVRS wheel scroll 경로가 꺼져 본문 스크롤이 막힌 것처럼 보인다.
- 이미지 paste 또는 포맷 command 뒤 C++이 계산한 editor HTML 결과는 공개 `LV.TextEditor.text`/`cursorPosition`
  API로 반영한다. RichText 문서 교체 직후 커서가 초기 위치로 되돌아가지 않도록 즉시 한 번, 다음 QML tick에서
  한 번 더 공개 cursor API로 복원한다. 이미지가 아닌 paste는 공개 `paste()` API로 되돌린다.
- 내부 `TextDocumentModel`이나 제거된 `editorImeAdapter` objectName에는 의존하지 않는다.
- `.wsnbody` XML 컨테이너 자체를 이 파일에 직접 연결하지 않는다.
- `LV.CodeEditor`, raw `TextEdit`, RichText overlay, parser/projection/rendering bridge를 추가하지 않는다.
