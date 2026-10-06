# `src/extension/trial/WhatSonTrialWshubAccessBackend.hpp`

<a id="role"></a>

## 역할
로컬 90평가판이 만료된 후 `.wshub` 액세스를 차단하는 선택적 백엔드를 선언합니다.

<a id="public-api"></a>

## 공개 API
- `evaluateAccess(...)`: 로컬 `.wshub` 경로 또는 `.wshub` 패키지를 가리키는 문서 URI에 대한 구조화된 결정을 반환합니다.
- `canAccess(...)`: 거부 메시지도 노출하는 편리한 부울 래퍼입니다.

<a id="decision-model"></a>

## 의사결정 모델
- `.wshub`가 아닌 대상은 무시되고 허용된 상태로 유지됩니다.
- `WhatSonRegisterManager::authenticated()`가 `true`인 경우 백엔드는 모든 평가판별 제한 사항을 우회합니다.
- `.wshub` 대상은 `WhatSonTrialActivationPolicy`가 활성 평가판 상태를 보고하는 동안에만 허용됩니다.
- 만료된 결정에는 설치에서 파생된 평가판 상태와 사용자에게 표시되는 거부 문자열이 포함됩니다.
- 로컬 `.wshub` 대상도 `.whatson/trial_register.xml`를 검사합니다.
- 평가판 레지스터 파일이 누락되면 로컬 `.wshub` 대상이 거부됩니다.
- 레지스터 페이로드는 `key` 값을 신뢰하기 전에 먼저 무결성 확인을 통과해야 합니다.
- 레지스터 페이로드는 `deviceUUID` 및 `key`를 모두 읽지만 무결성 확인이 성공한 후에는 `key` 비교만 액세스를 제어합니다.
- 로컬 허브 키가 유지된 인앱 평가판 키와 다른 경우 장치 UUID가 여전히 일치하더라도 결정이 거부됩니다.
