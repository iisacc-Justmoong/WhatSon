# `src/app/runtime/startup/WhatSonStartupHubResolver.hpp`

<a id="role"></a>

## 역할
이 헤더는 시작 허브 패키지와 필요한 탑재/액세스 상태를 확인하는 도우미 기능을 정의합니다.

<a id="interface-alignment"></a>

## 인터페이스 정렬
- `resolveStartupHubSelection(...)`는 `ISelectedHubStore`와 `WhatSonHubMountValidator`를 함께 수락하며, 스타트업 소스 정책을 소유합니다.
- `StartupHubSelection`는 지속된 시작 허브를 마운트할 수 없을 경우 선택적 실패 메시지와 함께 해결된 소스(`PersistedSelection`)를 보고합니다.
- 시작 허브 해상도는 더 이상 구체적인 설정 저장소 구현에 연결되지 않으며, 더 이상 패키징된 대체 경로 경로를 포함하지 않습니다.
