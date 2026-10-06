# `src/app/models/file/conflict/WhatSonTimestampConflictResolver.cpp`

<a id="runtime-behavior"></a>

## 런타임 동작

- 로컬 노트 타임스탬프 형식 `yyyy-MM-dd-hh-mm-ss` 및 Qt ISO 타임스탬프 변형을 구문 분석합니다.
- `mergeBodyByTimestamp(...)`는 기본 풀 타임스탬프 이후의 파일 시스템 발전을 충돌 트리거로 처리합니다.
- 충돌이 발생하면 파일 시스템과 수신 타임스탬프를 비교하고 최신 본문의 소스 텍스트를 반환합니다.
- 동일한 타임스탬프는 수신 본문을 유지하여 활성 편집기를 타이 브레이커로 저장합니다.
- `isTimestampNewer(...)`는 읽기 측 동기화 결정에 대해 동일한 엄격한 타임스탬프 비교를 제공하며, 예를 들어 유휴 파일 시스템 풀이 열린 편집기 세션을 대체할지 여부를 결정합니다.

<a id="tests"></a>

## 테스트

- `test/cpp/suites/timestamp_conflict_resolver_tests.cpp`는 resolver 전용 승자 선택, 엄격한 최신 타임스탬프 검사, 그리고 파일 시스템 최신 사례와 신규 수신 사례 모두에 대한 파일 저장소 통합을 포함합니다.
