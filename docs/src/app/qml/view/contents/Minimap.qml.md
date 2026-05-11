# `src/app/qml/view/contents/Minimap.qml`

## Responsibility

`Minimap.qml` is the only contents-side minimap view. It renders a VSCode-style right-side miniature of the current
LVRS rich-text editor document from view inputs.

## Contract

- Root type: `Item`.
- Imports: `QtQuick` and `LVRS 1.0 as LV`.
- It exposes only view inputs: `documentText`, source content geometry, source font tokens, viewport colors, and a
  `scrollTarget` callback supplied by the sibling `TextEditor.qml`.
- It renders `documentText` through a read-only `TextEdit.RichText` preview scaled down by the editor content width and
  document height, matching VSCode's right minimap model instead of drawing abstract row markers.
- Its minimap width and maximum preview scale are reduced by the fixed `0.85` size factor so the right-side preview is
  15% smaller than the base VSCode-style surface used by this app.
- It draws the current editor viewport as an overlay thumb only while the minimap is hovered or actively dragged, and
  forwards click/drag gestures to `scrollTarget`.
- It does not calculate document projection, parser output, persistence, or editor session state.

## 한국어

- 기준: contents 내부 QML에서 허용되는 세 뷰 중 미니맵 담당 파일이다.
- 동작: sibling `TextEditor.qml`이 전달한 rich text document와 viewport geometry를 축소 렌더링하고, 현재
  viewport thumb을 VSCode 오른쪽 미니맵처럼 hover/drag 중에만 표시한다.
- 동작: 미니맵 폭과 최대 preview scale은 `0.85` 크기 계수로 고정해 이전 기준보다 15% 작게 표시한다.
- 동작: 미니맵 click/drag는 `scrollTarget` callback을 통해 editor viewport scroll만 요청한다.
- 금지: C++ metric backend, snapshot/projection/rendering/persistence wiring을 재도입하지 않는다.
