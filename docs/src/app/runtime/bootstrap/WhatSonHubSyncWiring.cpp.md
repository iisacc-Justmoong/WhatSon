# `src/app/runtime/bootstrap/WhatSonHubSyncWiring.cpp`

<a id="responsibility"></a>

## 책임
`main.cpp`에서 추출한 허브 동기화 배선을 구현합니다.

<a id="behavior"></a>

## 행동
- `syncFailed`에서 경고 출력까지 하나의 로깅 연결을 구축합니다.
- `hubFilesystemMutated() -> acknowledgeLocalMutation()`를 사용하여 제공된 소스 객체당 로컬 변이 연결을 하나 생성합니다.
- `HubSyncWiringResult`를 통해 집계 유효성을 보고합니다.
- 동기화 도메인 통합 후 `src/app/models/file/sync`의 `WhatSonHubSyncController`를 포함합니다.
- `main.cpp`는 이제 해당 소스 목록에 프로젝트 계층 구조 컨트롤러를 제공하므로, 프로젝트 계층 구조 쓰기는 외래 변경 재로드 경로를 트리거하는 대신 로컬 변이로 간주됩니다.

<a id="test-coverage"></a>

## 테스트 범위

자동 테스트 파일이 이 저장소에서 제거되었습니다. 시동/런타임 연기 실행을 통해 배선 유효성을 확인하십시오.
