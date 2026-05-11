# `src/app/qml/view/mobile/MobileEventSurface.qml`

## Responsibility

`MobileEventSurface.qml` is the mobile editor event surface. It remains a QML-owned transparent surface, while
touch/scroll/gesture classification is delegated to C++ `MobileEventSurfaceController`.

## Contract

- Root type: `Item`.
- Imports: `QtQuick`, `WhatSon.App.Internal 1.0`, and `LVRS 1.0 as LV`.
- It exposes `canvasColor`, `eventSurfaceEnabled`, and a `controller` alias for the internal
  `MobileEventSurfaceController`.
- It owns only the QML touch surface and point snapshot forwarding. `MultiPointTouchArea` forwards begin/update/end/cancel
  snapshots to C++.
- It relays C++ recognition signals as `touchClassified(...)`, `scrollClassified(...)`, `gestureClassified(...)`, and
  `classificationChanged(...)`.
- It does not own routing, event persistence, calendar models, note sessions, editor mutation, or classification policy.

## 한국어

- 기준: 모바일 노트 에디터의 touch event surface이며 QML 리소스와 qmlcache 생성에서 항상 유효해야 한다.
- 동작: `MultiPointTouchArea`가 touch point snapshot만 만들고 C++ `MobileEventSurfaceController`에 전달한다.
- 동작: touch, scroll(direction 포함), gesture(finger count 및 direction 포함) 판별은 C++이 수행한다.
- 금지: routing, event persistence, calendar model, note/editor session, classification policy 책임을 이 파일에
  추가하지 않는다.
