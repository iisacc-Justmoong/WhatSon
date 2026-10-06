# `src/app/models/file/statistic/WhatSonNoteFileStatSupport.hpp`

<a id="responsibility"></a>

## 책임

`WhatSonNoteFileStatSupport`는 노트 열기 추적에 사용되는 노트 헤더 통계 동기화 도우미를 노출합니다.

<a id="public-api"></a>

## 공개 API

- `incrementOpenCountForNoteHeader(...)` : 메모 선택 추적을 위해 지속된 `.wsnhead` 오픈 카운트 메타데이터만 재작성합니다. 도우미는 또한 지속된 `lastOpenedAt` 타임스탬프를 새로 고칩니다.
- `refreshTrackedStatisticsForNote(...)` : 지속된 노트 헤더를 검증하고 선택적으로 `openCount` 및 `lastOpenedAt`를 증가시킬 수 있습니다.
- `refreshTrackedStatisticsForNoteId(...)` : 현재 허브 내부의 메모를 해결하고, 호출자가 해당 메모의 디렉터리 경로를 알 필요 없이 카운터를 새로 고칩니다.
