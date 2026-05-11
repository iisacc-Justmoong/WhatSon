# `src/app/qml/view/contents/TextEditor.qml`

## Responsibility

`TextEditor.qml` is the only contents-side text editor view. It wraps LVRS `TextEditor` directly.

## Contract

- Root type: `LV.TextEditor`.
- Imports: `QtQuick` and `LVRS 1.0 as LV`.
- `noteBodyFilePath` is the wrapper-owned input for the editor HTML session file prepared by C++.
- `filePath` is bound to `noteBodyFilePath`, letting LVRS perform file read/sync on the session file directly.
- `viewportContentY` relays the LVRS editor viewport scroll offset so the sibling gutter can keep line numbers aligned.
- `editorVisualLineHeight` derives the currently rendered line step from the public LVRS editor item
  `contentHeight / lineCount`, falling back to `LV.TextEditor.lineHeight` only before the surface has measurable
  content. The gutter consumes this value instead of the token line height so line-number spacing follows the body text.
- `preferNativeGestures` follows `LV.Theme.mobileTarget`. Desktop must keep LVRS wheel scrolling enabled even while
  the editor has focus; forcing mobile native gestures globally makes note body scrolling appear disabled.
- The wrapper uses only public LVRS `TextEditor` surface APIs for editor text, cursor movement, and paste forwarding.
  It must not reach into the internal `TextDocumentModel` or the removed `editorImeAdapter` object.
- Replacing the current document text for a C++-computed resource insertion assigns the C++-projected editor HTML to
  `LV.TextEditor.text` with the returned cursor position, then lets LVRS perform its automatic write-through sync.
- `editorReadOnly` lets the C++ note session freeze the native surface while no note is selected or a note source is
  loading.
- The file does not compute source mutations, resource tags, projection, rendering, persistence, tag management, or
  editor sessions.

## 한국어

- 기준: contents 내부 QML에서 허용되는 세 뷰 중 텍스트 에디터 담당 파일이다.
- 선택된 노트가 있으면 `noteBodyFilePath`를 통해 C++이 만든 editor HTML session file을 편집한다.
- 거터 동기화를 위해 editor viewport의 `contentY`와 실제 렌더 행 높이를 얇게 전달한다. 렌더 행 높이는
  LVRS editor item의 `contentHeight / lineCount`에서 계산하고, 측정 전에는 `LV.TextEditor.lineHeight`로
  되돌린다.
- `preferNativeGestures`는 `LV.Theme.mobileTarget`을 따른다. 데스크톱에서 이를 강제로 켜면 포커스 중
  LVRS wheel scroll 경로가 꺼져 본문 스크롤이 막힌 것처럼 보인다.
- 이미지 paste 뒤 C++이 계산한 editor HTML 결과는 공개 `LV.TextEditor.text`/`cursorPosition` API로 반영하고,
  이미지가 아닌 paste는 공개 `paste()` API로 되돌린다.
- 내부 `TextDocumentModel`이나 제거된 `editorImeAdapter` objectName에는 의존하지 않는다.
- `.wsnbody` XML 컨테이너 자체를 이 파일에 직접 연결하지 않는다.
- `LV.CodeEditor`, raw `TextEdit`, RichText overlay, parser/projection/rendering bridge를 추가하지 않는다.
