# `src/app/models/file/sync/WhatSonHubSyncObservationBuilder.hpp`

<a id="role"></a>

## 역할
재귀 허브 관찰 빌더를 선언합니다.

<a id="contract"></a>

## 계약
- `inspectHub(...)`는 하나의 `WhatSonHubSyncObservation`를 반환합니다.
- 빌더는 마운트된 허브 탐색, 서명 페이로드 구성 및 디렉터리 감시 경로 검색을 소유합니다.
- 런타임 상태를 다시 로드할지 여부를 결정하지 않으며 `QFileSystemWatcher` 경로를 직접 등록하지 않습니다.
