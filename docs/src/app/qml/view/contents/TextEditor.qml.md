# `src/app/qml/view/contents/TextEditor.qml`

## Responsibility

`TextEditor.qml` is the only contents-side text editor view. It wraps LVRS `TextEditor` directly.

## Contract

- Root type: `LV.TextEditor`.
- Imports: `QtQuick` and `LVRS 1.0 as LV`.
- `filePath` stays empty.
- The file does not expose compatibility properties for source snapshots, projection, rendering, persistence, tag
  management, or editor sessions.

## 한국어

- 기준: contents 내부 QML에서 허용되는 세 뷰 중 텍스트 에디터 담당 파일이다.
- `LV.CodeEditor`, raw `TextEdit`, RichText overlay, parser/projection/rendering bridge를 추가하지 않는다.
