# `src/app/qml/view/contents/Gutter.qml`

## Responsibility

`Gutter.qml` is the only contents-side gutter view. It is a QML-only visual rail for line numbers and the editor
separator.

## Contract

- Root type: `Item`.
- Imports: `QtQuick` and `LVRS 1.0 as LV`.
- It exposes only view inputs: `lineCount`, `lineHeight`, `contentY`, colors, and `showLineNumbers`.
- It does not own parser, projection, persistence, editor session, or note mutation behavior.

## 한국어

- 기준: contents 내부 QML에서 허용되는 세 뷰 중 거터 담당 파일이다.
- 금지: source snapshot, projection, rendering pipeline, persistence, editor backend wiring을 추가하지 않는다.
