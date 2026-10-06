# `src/app/models/file/sync/WhatSonHubSyncScheduler.hpp`

<a id="role"></a>

## 역할
허브 동기화 확인을 위한 타이머 스케줄러를 선언합니다.

<a id="contract"></a>

## 계약
- 주기적인 폴링 및 디바운스 억제 기능을 보유하고 있습니다.
- 디바운싱된 동기화 확인을 실행해야 할 때 `syncCheckDue()`를 내보냅니다.
- `QTimer` 소유권을 노출하지 않고 `WhatSonHubSyncController`에서 사용하는 간격 설정기를 제공합니다.
