# `src/app/models/file/statistic/WhatSonNoteFileStatSupport.cpp`

<a id="responsibility"></a>

## 책임

이 도우미는 나머지 헤더 전용 노트 열기 통계 규칙을 소유합니다.

<a id="header-counters"></a>

## 헤더 카운터

- `openCount`
- `lastOpenedAt`

<a id="tracking-rules"></a>

## 추적 규칙

- `incrementOpenCountForNoteHeader(...)`는 이제 저렴한 노트 선택 경로입니다. `.wsnhead` 메타데이터만 재작성하고 본문 파싱 및 허브 전역 백링크 스캔은 건너뜁니다.
- 저렴한 헤더 전용 경로와 전체 `refreshTrackedStatisticsForNote(..., true)` 경로 모두 현재 UTC ISO 타임스탬프를 `openCount`로 진행할 때마다 `lastOpenedAt`를 스탬프합니다.
- `openCount`는 호출자가 명시적으로 추적-스탯 새로 고침을 원할 때 `refreshTrackedStatisticsForNote(..., true)`를 통해 여전히 증가합니다.
- 도우미는 의도적으로 `modifiedCount`를 변형하지 않습니다. 해당 카운터를 소유한 경로를 작성합니다.
