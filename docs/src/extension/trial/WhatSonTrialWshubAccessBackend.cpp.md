# `src/extension/trial/WhatSonTrialWshubAccessBackend.cpp`

<a id="role"></a>

## 역할
선택적 평가판 확장을 위해 `.wshub` 액세스 게이트를 구현합니다.

<a id="behavior"></a>

## 행동
- 로컬 파일 시스템 경로는 `QDir::cleanPath(...)`로 정규화됩니다.
- 백엔드는 로컬 `.wshub` 디렉터리와 `.wshub`로 끝나는 URI 경로를 모두 인식합니다.
- `WhatSonRegisterManager::authenticated()`가 `true`인 경우 백엔드는 시험 등록 확인이 실행되기 전에 허용된 결정을 반환합니다.
- 평가판이 만료되면 백엔드는 액세스를 거부하고 마지막 활성 날짜를 ISO 형식으로 보고합니다.
- 로컬 `.wshub` 경로는 `.whatson/trial_register.xml`를 읽고, HMAC 서명을 확인한 다음, `key` 값을 유지된 인앱 평가판 키와 비교합니다.
- `.whatson/trial_register.xml`가 누락된 경우 키 비교를 완료할 수 없기 때문에 로컬 `.wshub` 경로도 거부됩니다.
- 누락되거나 잘못된 형식의 로컬 레지스터 파일 키는 파일이 존재할 때 액세스를 거부합니다.
- 일반 텍스트 `key` 값이 유효한 것처럼 보이더라도 서명 불일치로 인해 액세스도 거부됩니다.
- XML 페이로드는 로컬에서 읽을 수 없기 때문에 문서 URI는 만료 전용으로 유지됩니다.
- 완전성을 위해 레지스터 `deviceUUID`가 로드되지만 액세스 결정에는 영향을 미치지 않습니다.

<a id="integration-intent"></a>

## 통합 의도
- 모듈은 `src/extension/trial` 아래에 있으므로 하위 소비 측 시작/로드 흐름은 확장을 필수 코어 종속성으로 전환하지 않고도 선택할 수 있습니다.
