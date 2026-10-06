# `src/app/permissions/WhatSonPermissionBootstrapper.cpp`

<a id="responsibility"></a>

## 책임
시작 시간 권한 요청 순서 지정 및 지속적인 결정 추적을 구현합니다.

<a id="key-behaviors"></a>

## 주요 행동
- `QSettings`를 통해 권한별 결정을 로드하고 유지합니다.
  - `permissions/<permissionId>/requested`
  - `permissions/<permissionId>/granted`
- 후속 부팅 시 이미 결정된 권한을 건너뜁니다.
- 각 권한을 비동기식으로 실행하고 `QTimer::singleShot(0, ...)`로 계속됩니다.
- 최종 집계 부여 상태를 계산하고 `permissions/granted`에 저장합니다.

<a id="platform-branches"></a>

## 플랫폼 지점
- Qt   런타임  권한은 `QT_CONFIG(permissions)` 가 활성화될 때만 요청됩니다. `WHATSON_DISABLE_QT_PERMISSION_REQUESTS=1` 은 런타임 앱에서 주입되며, CMake 에서 Qt 플러그인이 시뮬레이터 연결 안정성을 위해 제외될 때 `permissions` 됩니다.

<a id="startup-integration"></a>

## 스타트업 통합
- `main.cpp`로 제작되었습니다.
- `start()`를 통한 포그라운드 서비스 시작 경로에서 트리거됩니다.

<a id="regression-checklist"></a>

## 회귀 체크리스트
- 기존 승인/거부 결정은 중복 프롬프트 없이 존중되어야 합니다.
- 결정되지 않은 새로운 권한 단계는 정의된 순서로 실행되어야 합니다.
- 최종 `permissions/granted`는 구성된 모든 단계를 반영해야 합니다.
