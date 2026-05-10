# `src/app/qml/view/contents/TextEditor.qml`

## Responsibility

`TextEditor.qml` is the only contents-side text editor view. It wraps LVRS `TextEditor` directly.

## Contract

- Root type: `LV.TextEditor`.
- Imports: `QtQuick` and `LVRS 1.0 as LV`.
- `noteBodyFilePath` is the wrapper-owned input for the parsed RAW source session file prepared by C++.
- `filePath` is bound to `noteBodyFilePath`, letting LVRS perform file read/sync on the session file directly.
- `viewportContentY` relays the LVRS editor viewport scroll offset so the sibling gutter can keep line numbers aligned.
- The wrapper uses only public LVRS `TextEditor` surface APIs for editor text, cursor movement, and paste forwarding.
  It must not reach into the internal `TextDocumentModel` or the removed `editorImeAdapter` object.
- Replacing the current document text for a C++-computed resource insertion assigns `LV.TextEditor.text` and cursor
  position, then lets LVRS perform its automatic write-through sync.
- `editorReadOnly` lets the C++ note session freeze the native surface while no note is selected or a note source is
  loading.
- The file does not compute source mutations, resource tags, projection, rendering, persistence, tag management, or
  editor sessions.

## 한국어

- 기준: contents 내부 QML에서 허용되는 세 뷰 중 텍스트 에디터 담당 파일이다.
- 선택된 노트가 있으면 `noteBodyFilePath`를 통해 C++이 만든 parsed RAW source session file을 편집한다.
- 거터 동기화를 위해 editor viewport의 `contentY`만 얇게 전달한다.
- 이미지 paste 뒤 C++이 계산한 source 결과는 공개 `LV.TextEditor.text`/`cursorPosition` API로 반영하고,
  이미지가 아닌 paste는 공개 `paste()` API로 되돌린다.
- 내부 `TextDocumentModel`이나 제거된 `editorImeAdapter` objectName에는 의존하지 않는다.
- `.wsnbody` XML 컨테이너 자체를 이 파일에 직접 연결하지 않는다.
- `LV.CodeEditor`, raw `TextEdit`, RichText overlay, parser/projection/rendering bridge를 추가하지 않는다.
