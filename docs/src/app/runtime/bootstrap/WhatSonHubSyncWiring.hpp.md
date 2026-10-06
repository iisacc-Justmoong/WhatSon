# `src/app/runtime/bootstrap/WhatSonHubSyncWiring.hpp`

<a id="responsibility"></a>

## 책임
`WhatSonHubSyncController`에 대한 부트스트랩 배선 도우미를 선언합니다.

<a id="public-contract"></a>

## 공공 계약
- `HubSyncWiringResult`: 생성된 연결 핸들 및 집계 배선 상태를 캡처합니다.
- `wireHubSyncController(...)`: 연결:
  - 컨트롤러 `syncFailed` 로깅 경로
  - 각 변이 소스 `hubFilesystemMutated()` 신호를 컨트롤러 `acknowledgeLocalMutation()`에 보냅니다.

<a id="design-intent"></a>

## 디자인 의도
`main.cpp`는 반복적인 신호 배선 세부 사항을 중복해서는 안 됩니다. 이 도우미는 런타임 동작을 변경하지 않고 유지하면서 연결 설정을 중앙 집중화합니다.
