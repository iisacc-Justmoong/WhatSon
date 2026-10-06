# `src/app/runtime/startup/WhatSonStartupHubResolver.cpp`

<a id="implementation-notes"></a>

## 구현 노트
- 이제 시작 선택 확인이 `ISelectedHubStore`를 통해 지속적인 허브 상태를 읽습니다.
- 개별 허브 마운트/액세스 사전 검사는 `WhatSonHubMountValidator`에 위임됩니다.
- 지속형 시작 허브가 없으면 확인자는 빈 선택 항목을 반환하고 시작은 온보딩 상태를 유지합니다.
- 지속된 시작 허브가 존재하지만 마운트할 수 없는 경우, 리졸버는 아무런 알림 없이가 모든 대체 경로 허브로 전환하는 대신 해당 실패를 유지합니다. 온보딩 표면은 실제 시작 오류를 표시할 수 있습니다.
- 개별 허브 후보에 대한 마운트 및 검증 동작은 이제 온보딩에서 사용되는 공유 허브 검증기에서 비롯됩니다.
