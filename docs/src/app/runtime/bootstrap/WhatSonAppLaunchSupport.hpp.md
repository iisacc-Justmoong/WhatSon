# `src/app/runtime/bootstrap/WhatSonAppLaunchSupport.hpp`

<a id="role"></a>

## 역할
`main.cpp`가 시작 정책을 인라인으로 다시 인코딩하지 않고도 사용할 수 있는 부트스트랩 시간 시작 옵션 도우미를 선언합니다.

<a id="implementation-notes"></a>

## 구현 노트
- `startupWorkspaceReady(...)`는 이제 지속된 허브가 성공적으로 마운트되면 시작 시 온보딩을 건너뛸 수 있는 규칙을 중앙 집중화합니다.
- 첫 번째 런타임 로드는 작업 공간 루트가 LVRS `AfterFirstIdle`에 도달한 후에 예약되며, 따라서 도메인 I/O 및 스냅샷 애플리케이션이 초기 창 생성을 차단하지 않습니다.
- 도우미는 `main.cpp`에 의존하지 않고 C++ 회귀 제품군에서 시작 대체 경로 정책을 테스트할 수 있도록 유지합니다.
