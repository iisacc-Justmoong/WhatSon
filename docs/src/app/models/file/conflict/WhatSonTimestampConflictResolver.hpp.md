# `src/app/models/file/conflict/WhatSonTimestampConflictResolver.hpp`

<a id="responsibility"></a>

## 책임

파일 충돌 검사에 사용되는 타임스탬프 최신성 도우미를 선언합니다.

<a id="contract"></a>

## 계약

- 엄격한 읽기 측 신선도 확인을 위해 `isTimestampNewer(...)`를 노출합니다.

<a id="boundary"></a>

## 경계

- 확인자는 노트 페이로드를 구문 분석하거나 헤더를 변경하거나 파일을 유지하지 않습니다.
- 파일 IO 및 버전 캡처는 이 모듈 외부에 있습니다.
