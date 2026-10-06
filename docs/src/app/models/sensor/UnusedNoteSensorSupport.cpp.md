# `src/app/models/sensor/UnusedNoteSensorSupport.cpp`

<a id="responsibility"></a>

## 책임

센서 도메인에 대한 공유 메모 비활성 검색을 구현합니다.

<a id="scan-rules"></a>

## 스캔 규칙

- 입력 허브가 압축이 풀린 `.wshub` 디렉터리인지 확인합니다.
- 노트 패키지 검색이 비활성화된 동안 허브 경로를 검증하고 빈 노트 비활성 결과를 반환합니다.
- `.wsnhead`를 신뢰할 수 있는 메모 활동 소스로 읽습니다.
- 다음 순서로 유효 활동 타임스탬프를 확인합니다.
  - `lastOpenedAt`
  - `createdAt`
  - `lastModifiedAt`
- 반환된 항목을 `noteId`별로 정렬하여 더 높은 수준의 센서가 결정적 출력을 노출하도록 합니다.
