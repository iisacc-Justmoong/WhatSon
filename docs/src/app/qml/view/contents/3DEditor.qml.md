# `src/app/qml/view/contents/3DEditor.qml`

## Role

Temporary build-safe placeholder for a future 3D resource contents view.

It is intentionally a plain Qt Quick `Item` with an object name only. It imports LVRS to stay inside the contents-view
QML contract, but it does not mount resource rendering, editor state, persistence, parser, or mutation wiring.

## 한국어

- 현재는 QML module 빌드 실패를 막기 위한 단순 `Item` placeholder다.
- `ContentViewLayout.qml`의 composition 대상이 아니며, 3D resource viewer 동작을 구현하지 않는다.
