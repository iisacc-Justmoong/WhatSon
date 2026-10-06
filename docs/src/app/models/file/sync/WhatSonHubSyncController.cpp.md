# `src/app/models/file/sync/WhatSonHubSyncController.cpp`

<a id="implementation-summary"></a>

## 구현 요약
구현은 허브 동기화를 3 명시적 단계로 변환합니다.

1. `WhatSonHubSyncObservationBuilder`에게 마운트된 `.wshub`를 검사하도록 요청하세요.
2. `WhatSonHubSyncWatcher` 및 `WhatSonHubSyncScheduler`로부터 감시자 및 스케줄러 힌트를 받습니다.
3. 기준선만 새로 고칠지 아니면 주입된 런타임 다시 로드 콜백을 호출할지 결정

<a id="observation-model"></a>

## 관찰 모델
관찰은 이제 `WhatSonHubSyncObservationBuilder`에 있습니다.

빌더는 마운트된 허브를 재귀적으로 순회하며 해싱을 위한 정규화된 파일/디렉토리 기록을 수집하고, `WhatSonHubSyncWatcher` 에 등록해야 하는 워쳐 경로를 기록합니다.

관찰된 서명은 의도적으로 다음을 무시합니다.
- `.whatson`

이로 인해 런타임 다시 로드 정책에서 앱 개인 장부 변동이 유지됩니다.

이는 단일 재귀 관찰 패스에서 서명 해싱 및 감시자 적용 범위를 유지합니다.

<a id="watcher-model"></a>

## 감시자 모델
수집된 감시자 경로는 `WhatSonHubSyncWatcher`에 공급됩니다.

이제 정규화된 감시 경로 세트가 변경되지 않으면 감시자 등록이 단락되므로 동일한 허브 토폴로지를 관찰하는 동기화 힌트가 더 이상 감시된 모든 경로를 해체하고 다시 추가하지 않습니다.

감시자는 다음을 소유한 `WhatSonHubSyncScheduler`와 쌍을 이룹니다.
- 최종 일관성을 위한 주기적인 타이머
- 버스트 억제를 위한 디바운스 타이머

따라서 컨트롤러는 모든 개별 파일 시스템 콜백을 즉시 런타임 재구축으로 전환하지 않고도 외부 편집에 계속 응답합니다.

<a id="local-mutation-handling"></a>

## 국소 돌연변이 처리
`acknowledgeLocalMutation()`는 다음으로 관찰된 서명 변경을 앱 소유로 표시합니다.

`onDebounceTimeout()`는 해당 앱 소유 변경 사항을 확인하면 기준을 새로 고치고 감시자 적용 범위를 다시 구축하지만 런타임 다시 로드 콜백을 호출하지 않습니다. 이는 사용자가 시작한 메모/폴더 쓰기가 자체 다시 로드를 통해 라이브 편집기 세션에서 반송되는 것을 방지하는 주요 분리입니다.

<a id="external-mutation-handling"></a>

## 외부 돌연변이 처리
로컬 돌연변이 승인 없이 서명이 변경되는 경우:
- 삽입된 다시 로드 콜백이 호출됩니다.
- 다시 로드에 실패하면 `syncFailed(...)`가 발생합니다.
- 성공적으로 다시 로드하면 동일한 관찰 페이로드를 재사용하여 기준을 새로 고치고 `syncReloaded(...)`를 내보냅니다.

<a id="architectural-note"></a>

## 건축 노트
이 구현은 더 이상 포인터, 터치 또는 애플리케이션 활성화 이벤트에 반응하지 않습니다. 허브 동기화는 감시자/타이머 힌트와 명시적인 로컬 쓰기 승인에 의해서만 이루어지며, 이는 파일 시스템 동기화 정책을 UI 탐색 및 동작 처리와 독립적으로 유지합니다.

<a id="tests"></a>

## 테스트
- `test/cpp/suites/hub_sync_controller_tests.cpp`는 분할 객체 경계를 다루고 관찰은 계약을 무시합니다.
- 회귀 체크리스트:
  - 런타임 배선(`main.cpp`, `WhatSonHubSyncWiring.cpp`)에는 `file/sync`의 이 구현이 포함되어야 합니다.
  - `src/app/models/file/sync`로의 경로 마이그레이션은 디바운스, 감시자 재구축 또는 로컬 돌연변이 우회 동작을 변경해서는 안 됩니다.
