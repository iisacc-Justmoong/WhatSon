# `src/app/runtime/bootstrap/WhatSonQmlLaunchSupport.hpp`

<a id="role"></a>

## 역할
LVRS `appentry` API가 지원하는 WhatSon 관련 QML 루트 로딩 도우미를 선언합니다.

<a id="public-helpers"></a>

## 공공 도우미
- `loadQmlRoot(...)`: 임의의 QML 모듈 루트를 로드하고 LVRS `QmlRootLoadResult`를 반환합니다.
- `loadWhatSonAppRoot(...)` : `WhatSon.App` QML 모듈에서 루트 객체를 로드하고 LVRS 루트/윈도우 결과를 보존합니다.
- `loadMainWindowRoot(...)`: 전체 LVRS 로드 결과가 포함된 기본 `Main` 루트에 대한 편의 래퍼입니다.
- `lastRootObject(...)`: 여전히 `QObject*`가 필요한 레거시 호출 사이트에 대해 마지막으로 생성된 루트 개체를 추출합니다.
- `loadQmlRootObject(...)`, `loadWhatSonAppRootObject(...)`, 및 `loadMainWindow(...)`: 결과 반환 헬퍼 주변의 호환성 래퍼.

<a id="policy"></a>

## 정책
호출자는 `show()`, `raise()` 또는 `requestActivate()`를 수동으로 호출하는 대신 `lvrs::QmlWindowActivationPolicy`를 전달합니다. 이렇게 하면 루트 생성 및 활성화가 LVRS 런타임 부트스트랩 계약에 맞춰 유지됩니다.

새로운 런타임 경로는 원시 `QObject*`에서 루트/창 상태를 다시 작성하는 대신 `QmlRootLoadResult`를 유지하고 이를 LVRS 수명 주기/포그라운드 서비스 도우미에 공급해야 합니다.
