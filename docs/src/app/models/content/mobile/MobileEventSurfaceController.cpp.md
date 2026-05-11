# `src/app/models/content/mobile/MobileEventSurfaceController.cpp`

## Responsibility

Implements mobile editor touch classification for the QML `MobileEventSurface`.

## Runtime Flow

1. QML calls `nextSessionId()` and `beginTouchSequence(...)` with the initial touch-point snapshot.
2. `updateTouchSequence(...)` compares the current centroid with the starting centroid.
3. A single finger that crosses the scroll threshold becomes `scroll`, with direction `up`, `down`, `left`, or `right`.
4. Two or more fingers that cross the gesture threshold become `gesture`, with both direction and `fingerCount`.
5. A pending sequence that ends inside the movement threshold becomes `touch`.
6. `cancelTouchSequence(...)` clears active state without emitting a touch/scroll/gesture recognition signal.

## Guardrails

- This class does not perform route changes, editor mutations, or note-session persistence.
- Direction means pointer movement direction, not content movement inversion.
- QML remains a surface and forwarding layer; it must not duplicate the classification thresholds or state machine.

## 한국어

- 기준: 모바일 노트 에디터 입력에서 touch, scroll, gesture를 분간하는 구현이다.
- 동작: 단일 손가락 이동은 scroll, 두 손가락 이상 이동은 gesture, 임계값 안에서 끝난 입력은 touch로 분류한다.
- 동작: scroll/gesture payload에는 방향과 finger count가 포함된다.
- 금지: route 전환, note mutation, editor persistence 책임을 이 객체에 섞지 않는다.
