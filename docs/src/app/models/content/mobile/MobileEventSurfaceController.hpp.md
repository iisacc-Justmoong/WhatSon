# `src/app/models/content/mobile/MobileEventSurfaceController.hpp`

## Responsibility

Declares the C++ classifier used by `MobileEventSurface.qml` to distinguish mobile editor touch input.

## Contract

- Exposes a QObject-backed QML type through the internal WhatSon QML registrar.
- Owns the event session id, active event kind, direction, finger count, and classification thresholds.
- Public invokable hooks are limited to `beginTouchSequence(...)`, `updateTouchSequence(...)`,
  `endTouchSequence(...)`, `cancelTouchSequence(...)`, `nextSessionId()`, and `resetState()`.
- Emits `touchRecognized(...)`, `scrollRecognized(...)`, and `gestureRecognized(...)` with a `QVariantMap` payload that
  includes `kind`, `direction`, `fingerCount`, centroid delta, phase, validity, and session id.
- The QML surface may forward point snapshots, but classification policy stays in this C++ object.

## 한국어

- 기준: 모바일 노트 에디터 입력 표면에서 touch, scroll, gesture를 구분하는 C++ 선언이다.
- 동작: QML은 point snapshot만 전달하고, 세션, 방향, finger count, 임계값 기반 분류는 이 객체가 소유한다.
- 금지: QML에 touch/scroll/gesture 판별 정책을 두지 않는다.
