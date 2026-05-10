# `src/app/qml/view/contents/TextEditor.qml`

## Responsibility

`TextEditor.qml` is the only contents-side text editor view. It wraps LVRS `TextEditor` directly.

## Contract

- Root type: `LV.TextEditor`.
- Imports: `QtQuick` and `LVRS 1.0 as LV`.
- `noteBodyFilePath` is the wrapper-owned input for the parsed RAW source session file prepared by C++.
- `filePath` is bound to `noteBodyFilePath`, letting LVRS perform file read/sync on the session file directly.
- `viewportContentY` relays the LVRS editor viewport scroll offset so the sibling gutter can keep line numbers aligned.
- `documentModel` and `imeAdapter` are discovered from LVRS object names only to support two narrow editor hooks:
  replacing the current document text with a C++-computed resource insertion result, and forwarding non-image paste
  back to the native LVRS/Qt paste adapter.
- `editorReadOnly` lets the C++ note session freeze the native surface while no note is selected or a note source is
  loading.
- The file does not compute source mutations, resource tags, projection, rendering, persistence, tag management, or
  editor sessions.

## 한국어

- 기준: contents 내부 QML에서 허용되는 세 뷰 중 텍스트 에디터 담당 파일이다.
- 선택된 노트가 있으면 `noteBodyFilePath`를 통해 C++이 만든 parsed RAW source session file을 편집한다.
- 거터 동기화를 위해 editor viewport의 `contentY`만 얇게 전달한다.
- 이미지 paste 뒤 C++이 계산한 source 결과를 LVRS document에 반영하고, 이미지가 아닌 paste는 native adapter로
  되돌리는 얇은 hook만 둔다.
- `.wsnbody` XML 컨테이너 자체를 이 파일에 직접 연결하지 않는다.
- `LV.CodeEditor`, raw `TextEdit`, RichText overlay, parser/projection/rendering bridge를 추가하지 않는다.
