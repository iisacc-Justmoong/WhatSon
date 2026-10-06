# `src/extension/trial/WhatSonTrialActivationPolicy.hpp`

<a id="role"></a>

## 역할
선택적 평가판 자격 흐름에 대한 90일 활성화 정책을 선언합니다.

<a id="public-surface"></a>

## 공공 표면
- `WhatSonTrialActivationState`: 설치 날짜, 마지막 활성 날짜, 경과 일수, 남은 일수, 활성 플래그 및 인증 우회 플래그가 포함된 불변 스타일 스냅샷.
- `refreshForDate(...)`: 제공된 달력 날짜에 대한 정책을 평가하고 캐시된 상태를 업데이트합니다.
- `refresh()`: `QDate::currentDate()`에 대해 평가하는 슬롯 지향 런타임 진입점입니다.
- `stateChanged()`: 캐시된 활성화 상태가 변경될 때만 발생합니다.

<a id="qmlruntime-use"></a>

## QML/런타임 사용
- 클래스는 `QObject`이므로 앱은 나중에 API를 다시 디자인하지 않고도 QML 또는 시작 오케스트레이션에 노출할 수 있습니다.
- `refresh()`는 의도된 시작 후크입니다. 한 번 호출한 다음 `active`에서 앱 활성화를 시작합니다.
- `WhatSonRegisterManager::authenticated()`가 `true`이면 `active`는 `true`로 유지되고 `bypassedByAuthentication`는 `true`가 되므로 하위 소비 측 코드는 평가판 만료를 무시할 수 있습니다.
