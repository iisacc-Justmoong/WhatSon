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
- `editorMeasuredContentHeight` subtracts that artificial bottom padding from the rich-text content height before
  calculating fallback visual line height, keeping the sibling gutter line spacing tied to real text lines.
- `editorRenderedLineCount` exposes the public LVRS editor item's rendered `lineCount` so the sibling gutter can create
  line-number rows for the actual visual editor surface instead of stopping at the parsed RAW source line count.
- `editorCursorLineIndex` is derived from the public LVRS `editorItem.cursorRectangle` and visual line metrics, giving
  the sibling gutter the current rendered cursor line for its visual indicator. Plain text/cursor-position counting is
  only a fallback because the editor document text is RichText HTML and line breaks can be represented as `<br/>`.
- `editorSelectionStart` and `editorSelectionLength` expose normalized public LVRS selection metadata so the outer
  content layout can dispatch C++ format commands without installing key handlers inside this wrapper.
- `scrollEditorViewportTo(contentY)` is a view-local hook used by the minimap to request a viewport scroll without
  introducing an editor backend object.
- `editorLineMetricsFor(lineIndex)` finds the peer visual line on the public LVRS `editorItem` using
  `positionAt(...)` and `positionToRectangle(...)`, then returns the editor-relative `y` and `height` for that line.
- `editorLineMetricsRevision` bumps when the editor text, width, line count, or rendered content height changes so the
  sibling gutter can re-evaluate per-line geometry.
- `editorVisualLineHeight` remains only the fallback line step for times when the LVRS editor item has not exposed a
  measurable peer line yet.
- `preferNativeGestures` follows `LV.Theme.mobileTarget`. Desktop must keep LVRS wheel scrolling enabled even while
  the editor has focus; forcing mobile native gestures globally makes note body scrolling appear disabled.
- The wrapper uses only public LVRS `TextEditor` surface APIs for editor text, cursor movement, and paste forwarding.
  It must not reach into the internal `TextDocumentModel` or the removed `editorImeAdapter` object.
- Replacing the current document text for a C++-computed resource or format insertion assigns the C++-projected editor
  HTML to `LV.TextEditor.text` with the returned cursor position, then lets LVRS perform its automatic write-through
  sync.
- `editorReadOnly` lets the C++ note session freeze the native surface while no note is selected or a note source is
  loading.
- The file does not compute source mutations, resource tags, projection, rendering, persistence, tag management, or
  editor sessions.

## 한국어

- 기준: contents 내부 QML에서 허용되는 세 뷰 중 텍스트 에디터 담당 파일이다.
- 선택된 노트가 있으면 `noteBodyFilePath`를 통해 C++이 만든 editor HTML session file을 편집한다.
- 거터 동기화를 위해 editor viewport의 `contentY`와 line-index 기반 metric provider를 얇게 전달한다. 각 줄
  번호는 `editorLineMetricsFor(lineIndex)`를 통해 실제 LVRS editor line의 위치와 높이를 받아 맞춘다.
- 거터가 생성할 줄 번호 개수는 `editorRenderedLineCount`로 제공한다. 이 값은 C++ session의 parsed RAW source
  line count가 아니라 LVRS editor surface의 실제 렌더 라인 수다.
- 현재 cursor line indicator를 위해 공개 LVRS `editorItem.cursorRectangle`과 visual line metric으로 계산한
  `editorCursorLineIndex`도 거터에 전달한다. RichText HTML에서는 줄바꿈이 `<br/>`로 표현될 수 있으므로
  HTML 문자열의 `\n` 개수만으로 현재 줄을 계산하지 않는다.
- 포맷 단축키 dispatch를 위해 공개 selection 상태를 정규화한 `editorSelectionStart`와 `editorSelectionLength`를
  바깥 layout에 전달한다.
- 미니맵 동기화를 위해 editor viewport의 폭/높이/contentHeight와 `scrollEditorViewportTo(contentY)` hook을
  제공한다.
- 본문 하단에는 viewport 높이의 50%에 해당하는 `bottomPadding`을 공개 LVRS editor item에 적용해 마지막 줄도
  화면 중앙 부근까지 끌어올려 볼 수 있게 한다. 이 인공 여백은 fallback line-height 계산에서 제외한다.
- `preferNativeGestures`는 `LV.Theme.mobileTarget`을 따른다. 데스크톱에서 이를 강제로 켜면 포커스 중
  LVRS wheel scroll 경로가 꺼져 본문 스크롤이 막힌 것처럼 보인다.
- 이미지 paste 또는 포맷 command 뒤 C++이 계산한 editor HTML 결과는 공개 `LV.TextEditor.text`/`cursorPosition`
  API로 반영하고, 이미지가 아닌 paste는 공개 `paste()` API로 되돌린다.
- 내부 `TextDocumentModel`이나 제거된 `editorImeAdapter` objectName에는 의존하지 않는다.
- `.wsnbody` XML 컨테이너 자체를 이 파일에 직접 연결하지 않는다.
- `LV.CodeEditor`, raw `TextEdit`, RichText overlay, parser/projection/rendering bridge를 추가하지 않는다.
