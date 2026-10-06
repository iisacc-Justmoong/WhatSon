# `src/app/models/file/sync/WhatSonHubSyncController.hpp`

<a id="role"></a>

## 역할
`WhatSonHubSyncController`는 마운트된 `.wshub` 파일 시스템과 메모리 내 런타임 간의 앱 수준 동기화 경계를 정의합니다.

이는 의도적으로 좁습니다. 클래스는 허브를 감시하고, 변경 힌트를 디바운스하고, 관찰된 허브 서명이 변경될 때 런타임 상태를 다시 빌드하도록 호출자 제공 다시 로드 콜백을 요청합니다.

구현은 협력자로 분할됩니다. 이제 컨트롤러는 공개 신호/슬롯 경계를 유지하고 재로드 결정 흐름을 소유하는 동시에 관찰, 감시자 등록 및 타이머 예약이 전용 개체에 적용됩니다.

<a id="public-api"></a>

## 공개 API
- `setReloadCallback(...)`: 관찰된 외부 변경 후에 사용되는 런타임 다시 로드 기능을 주입합니다.
- `setCurrentHubPath(...)` : 마운트된 허브 경로를 전환하고, 시그니처 베이스라인을 재구성하며, 워처 커버리지를 재구성합니다.
- `setPeriodicIntervalMs(...)` / `setDebounceIntervalMs(...)` : 런타임 진단 또는 플랫폼 조정에 대한 폴링/디바운스 정책을 조정합니다.
- `requestSyncHint()`: 디바운싱된 동기화 확인을 예약합니다.
- `acknowledgeLocalMutation()` : 앱 소유 쓰기를 표시하므로 다음 서명 변경 시 런타임를 다시 로드하는 대신 기준선을 새로 고칩니다.

<a id="signals"></a>

## 신호
- `syncReloaded(hubPath)`: 다시 로드 콜백이 성공하고 기준선이 새로 고쳐진 후에 발생합니다.
- `syncFailed(errorMessage)`: 다시 로드 콜백이 실패를 보고할 때 발생합니다.

<a id="architectural-constraints"></a>

## 건축적 제약
- 컨트롤러는 파일 시스템 지향입니다. 더 이상 애플리케이션 이벤트 첨부, 이벤트 필터링 또는 앱 활성화 훅을 노출하지 않습니다.
- UI 탐색, 제스처 및 일반 입력 이벤트는 이 클래스의 책임을 벗어납니다.
- 로컬 앱 쓰기는 `acknowledgeLocalMutation()`를 통해 외부 쓰기와 다르게 처리되며, 이는 현재 세션이 자체 변이 경로 이후에 다시 로드되는 것을 방지합니다.
- `WhatSonHubSyncObservationBuilder`는 단일 통과 관측 계약을 보유하고 있습니다: 하나의 재귀 보행은 시그니처 페이로드와 감시 경로 커버리지를 모두 생성합니다.
- `WhatSonHubSyncWatcher`는 마지막으로 적용된 감시 경로 설정을 기억하므로, 변경되지 않은 동기화 힌트가 감시자 등록을 재구성하지 않습니다.
- `WhatSonHubSyncScheduler`는 주기적인 폴링 및 디바운스 억제 기능을 보유하고 있습니다.

<a id="collaborators"></a>

## 협력자
- `main.cpp` : 컨트롤러를 생성하고, 리로드 콜백을 주입하며, 계층형 컨트롤러에서 로컬 변이 확인을 전송합니다.
- `WhatSonHubPathUtils`: 장착된 허브 경로를 정규화합니다.
- `WhatSonHubSyncObservationBuilder`: 장착된 허브를 검사하고 관측 시그니처를 구축합니다.
- `WhatSonHubSyncWatcher`: 재귀적 `QFileSystemWatcher` 적용 범위를 래핑합니다.
- `WhatSonHubSyncScheduler`: 주기적/디바운스 타이머를 래핑합니다.

<a id="tests"></a>

## 테스트
- `test/cpp/suites/hub_sync_controller_tests.cpp`는 책임 분담과 `.whatson` 관찰 무시 계약을 포함합니다.
- 회귀 체크리스트:
  - 포함 소비자는 `file/sync/WhatSonHubSyncController.hpp`를 통해 이 컨트롤러를 확인해야 합니다.
  - 이 헤더를 `file/sync`로 이동하면 공용 API 표면 또는 신호/슬롯 계약이 변경되어서는 안 됩니다.
