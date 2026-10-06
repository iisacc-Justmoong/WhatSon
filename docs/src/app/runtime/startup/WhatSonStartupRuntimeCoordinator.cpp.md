# `src/app/runtime/startup/WhatSonStartupRuntimeCoordinator.cpp`

<a id="implementation-notes"></a>

## 구현 노트
- 로더 사용은 이제 `m_parallelLoader`를 통해 인터페이스 기반으로 이루어집니다.
- 코디네이터는 로더 구현이 주입되지 않으면 로드를 거부합니다.
- 시작 시 더 이상 별도의 사전 윈도우 런타임 로드 또는 연기된 계층 구조 부트스트랩 경로가 없습니다. `main.cpp`는 라우팅을 위해 지속된 허브를 마운트한 다음, 첫 번째 워크스페이스 유휴 전환 후 정상적인 전체 런타임 부하를 스케줄링합니다.
- Hub- 런타임의 부작용(`libraryController.setHubStore(...)` 및 tag-depth 전파)은 이제 전체 요청된 로드가 성공한 후에만 적용되므로, 실패한 도메인 로드는 실시간 런타임 세션을 부분적으로 재타깃팅할 수 없습니다.
