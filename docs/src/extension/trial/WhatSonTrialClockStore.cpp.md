# `src/extension/trial/WhatSonTrialClockStore.cpp`

<a id="role"></a>

## 역할
선택적 평가판 모듈에 대한 타임스탬프 감사 지속성 경로를 구현합니다.

<a id="behavior"></a>

## 행동
- 종료 타임스탬프는 UTC로 정규화되고 밀리초 단위로 ISO-8601 텍스트로 유지됩니다.
- 저장소는 원시 마지막 종료 타임스탬프와 단조로운 최고 수위 표시를 모두 유지합니다.
- 현재 UTC 시간이 최고 워터마크보다 이전인 경우 `WhatSonTrialClockCheck`를 통해 롤백이 보고됩니다.

<a id="why-two-keys-exist"></a>

## 2 키가 존재하는 이유
- `lastExitTimestampUtc`는 가장 최근 종료에 대해 기록된 원시 시간을 보존합니다.
- `lastSeenTimestampUtc`는 롤백된 시스템 시계에 따라 나중에 종료가 발생하더라도 증거를 보존합니다.
