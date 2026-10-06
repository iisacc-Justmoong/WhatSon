# `src/app/runtime/threading/WhatSonRuntimeParallelLoader.cpp`

<a id="responsibility"></a>

## 책임

`WhatSonRuntimeParallelLoader.cpp`는 요청된 도메인 스냅샷 작업에 대해 LVRS `BootstrapParallelTask` 항목을 빌드하고 `lvrs::runBootstrapParallelTasks(...)`를 통해 실행한 다음 요청된 도메인 세트가 성공한 후 변경할 수 없는 스냅샷 페이로드를 기본 스레드 컨트롤러에 다시 적용합니다.

<a id="requested-domain-selection"></a>

## 요청된 도메인 선택

이제 로더는 명시적인 요청 도메인 마스크를 허용합니다. 따라서 시작 시 중요한 첫 번째 프레임 도메인만 로드하고 지연된 후속 작업을 위해 우선 순위가 낮은 계층만 남겨 둘 수 있습니다.

<a id="shared-library-snapshot-rule"></a>

## 공유 라이브러리 스냅샷 규칙

라이브러리와 북마크 도메인이 모두 존재하는 경우 이제 로더는 라이브러리 도메인을 한 번 인덱싱하고 해당 공유 라이브러리 스냅샷에서 북마크를 파생합니다.

이는 북마크 작업이 동일한 라이브러리 노트 세트를 독립적으로 다시 분석한 이전 중복 `.wshub` 순회를 제거합니다.

<a id="failure-behavior"></a>

## 실패 행동

- 라이브러리 스냅샷이 실패하면 파생된 책갈피 결과도 동일한 오류와 함께 실패합니다.
- 라이브러리 도메인이 없으면 로더는 독립형 북마크 스냅샷 경로로 대체됩니다.
- 로더는 이제 `hub.runtime`를 포함한 모든 요청된 도메인을 LVRS `BootstrapParallel`를 통해 스테이지하며, 모든 요청된 도메인이 성공한 경우에만 스냅샷을 라이브 컨트롤러/런타임 저장소에 다시 적용합니다.
- 요청된 도메인 중 하나라도 실패하면, 로더는 현재 런타임 상태를 부분적으로 변형시키지 않고 실패를 반환합니다.

<a id="test-coverage"></a>

## 테스트 범위

`test/cpp/suites/runtime_parallel_loader_tests.cpp`는 이 로더를 LVRS `BootstrapParallel`에 유지하고 직접 `QThread`/`QEventLoop` 작업자 관리가 반환되는 것을 방지하며 전부 아니면 전무 적용 게이트를 보존합니다.
