# `src/app/qml/view/contents/Minimap.qml`

## Responsibility

`Minimap.qml` is the only contents-side minimap view. It renders presentation-only row markers from view inputs.

## Contract

- Root type: `Item`.
- Imports: `QtQuick` and `LVRS 1.0 as LV`.
- It exposes only view inputs: `rowCount`, `rowWidthRatios`, `scrollProgress`, `viewportRatio`, and colors.
- It does not calculate document projection, visual line metrics, parser output, persistence, or editor session state.

## 한국어

- 기준: contents 내부 QML에서 허용되는 세 뷰 중 미니맵 담당 파일이다.
- 금지: C++ metric backend, snapshot/projection/rendering/persistence wiring을 재도입하지 않는다.
