# `src/app/qml/view/mobile/MobileEventSurface.qml`

## Responsibility

`MobileEventSurface.qml` is now an inert compatibility placeholder. The mobile editor route does not mount it, and the
file must not install touch, pointer, gesture, or LVRS event-listener surfaces.

## Contract

- Root type: `Item`.
- Imports: `QtQuick` and `LVRS 1.0 as LV`.
- `eventSurfaceEnabled` defaults to `false`.
- The root is `enabled: false`, `visible: false`, and `runtimeHitTransparent: true`.
- It must not instantiate `MobileEventSurfaceController`, `LV.EventListener`, `MultiPointTouchArea`, `MouseArea`,
  `TapHandler`, `DragHandler`, or `PinchHandler`.

## 한국어

- 기준: 모바일 입력 보정 surface는 현재 사용하지 않는다.
- 동작: 이 파일은 호환 placeholder이며, route에서 mount하지 않는다.
- 금지: touch, pointer, gesture, event listener, controller wiring을 추가하지 않는다.
