# `src/extension/trial/WhatSonTrialClockStore.hpp`

<a id="role"></a>

## 역할
시스템 시계가 앱 세션 간에 뒤로 이동했는지 여부를 감지하는 데 사용되는 로컬 타임스탬프 감사 저장소를 선언합니다.

<a id="public-api"></a>

## 공개 API
- `loadLastExitTimestampUtc()`: 가장 최근에 스탬프가 찍힌 앱 종료 UTC 타임스탬프를 반환합니다.
- `loadLastSeenTimestampUtc()`: 지금까지 관찰된 모든 종료 타임스탬프의 단조 높은 워터 마크를 반환합니다.
- `inspect(...)`: 제공된 UTC 시간을 최고 워터마크와 비교하고 롤백 메타데이터를 보고합니다.
- `stampExitTimestamp(...)`: 현재 종료 타임스탬프를 기록하고 기존 최고 워터마크와 새 값 중 더 큰 값을 유지합니다.
- `clear()`: 지속되는 타임스탬프 키를 모두 제거합니다.

<a id="persistence-keys"></a>

## 지속성 키
- `extension/trial/lastExitTimestampUtc`
- `extension/trial/lastSeenTimestampUtc`
