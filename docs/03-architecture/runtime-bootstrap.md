# Runtime Bootstrap 아키텍처 (Draft)

## 엔트리포인트

`src/app/main.cpp`

## 부트 순서(현재 구현)

1. `QGuiApplication` 초기화
2. 디버그 모드/환경 경로 설정
3. QML 엔진 생성
4. 핵심 객체 생성
    - 계층 ViewModel 8종
    - `SidebarHierarchyViewModel`, `PanelViewModelRegistry`
    - `WhatSonHubRuntimeStore`, `WhatSonAsyncScheduler`, `SystemCalendarStore`
5. blueprint에서 `.wshub` 자동 탐색
6. 병렬 도메인 로딩(`WhatSonRuntimeParallelLoader`)
7. QML context property 주입
8. `Main` 로드 후 스케줄러 시작
9. Permission bootstrap 체인 시작

## 설계 포인트

- Implemented: 병렬 로딩 + 메인스레드 적용 분리
- Implemented: sidebar 상태 단일화(`SidebarHierarchyViewModel`)
- Implemented: 정책 잠금(`ArchitecturePolicyLock`)을 통한 경계 고정
- Partial: 권한 흐름은 플랫폼별 차이가 크므로 운영 문서 보완 필요

## 실패 처리

- 도메인별 로딩 실패는 경고 및 trace를 남기고 다른 도메인 로딩을 지속한다.
- root QML 객체 생성 실패 시 앱은 즉시 실패 종료한다.
