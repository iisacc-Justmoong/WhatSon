# `src/app/runtime/startup/WhatSonStartupRuntimeCoordinator.hpp`

<a id="role"></a>

## 역할
`WhatSonStartupRuntimeCoordinator`는 `.wshub` 런타임 데이터를 도메인 컨트롤러로 로드하는 것을 조정합니다.

<a id="interface-alignment"></a>

## 인터페이스 정렬
- 런타임 대상은 이제 `IWhatSonRuntimeParallelLoader::Targets`를 재사용합니다.
- 코디네이터는 `setParallelLoader(...)`를 통해 로더를 승인합니다.
- 공개 시작 표면은 정상적인 전체 허브 로딩과 리소스 도메인 재로드로 축소되며, 워크스페이스 루트가 표시된 후 지속된 시작 스케줄링은 `main.cpp`가 소유합니다.
- 이전 지연된 사이드바 활성화 부트스트랩 경로가 제거되어 시작 시 사전 윈도우 부분 부하와 후속 계층 부하 대신 하나의 런타임 로드 경로를 가집니다.
