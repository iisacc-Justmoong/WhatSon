# `src/extension/trial/WhatSonTrialActivationPolicy.cpp`

<a id="role"></a>

## 역할
`WhatSonTrialInstallStore` 위에 기본 로컬 평가판 기간 계산을 구현합니다.

<a id="evaluation-rules"></a>

## 평가 규칙
- 저장된 값이 없으면 첫 번째 새로 고침 시 설치 날짜가 느리게 생성됩니다.
- `lastActiveDate`는 `installDate + 89 days`입니다.
- `elapsedDays`는 더 큰 자격 창을 생성하는 대신 네거티브 클럭 드리프트를 `0`로 고정합니다.
- `active`는 `true`이고 `elapsedDays < 90`입니다.
- 활성 상태에서는 `remainingDays`가 `90 - elapsedDays`이고, 그렇지 않으면 `0`입니다.
- `WhatSonRegisterManager::authenticated()`가 `true`인 경우 정책은 서명된 인증 마커가 성공적으로 확인된 후에만 인증 우회 상태를 보고합니다.

<a id="change-signaling"></a>

## 신호 변경
- `stateChanged()`는 새로 계산된 스냅샷이 캐시된 스냅샷과 다른 경우에만 생성됩니다.
- 이는 향후 UI 바인딩을 안정적으로 유지하고 같은 날 반복되는 새로 고침에 대한 중복 반응을 방지합니다.
