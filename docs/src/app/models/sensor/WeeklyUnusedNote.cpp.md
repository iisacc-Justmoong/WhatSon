# `src/app/models/sensor/WeeklyUnusedNote.cpp`

<a id="responsibility"></a>

## 책임

`UnusedNoteSensorSupport` 주위에 주간 비활성 래퍼를 구현합니다.

<a id="window-definition"></a>

## 창 정의

- 새로 고침당 하나의 UTC 참조 시간을 캡처합니다.
- `referenceUtc.addDays(-7)`를 주간 컷오프로 사용합니다.
- 공유 파싱, 숨김 경로 필터링 및 페이로드 구성을 `UnusedNoteSensorSupport::collectUnusedNoteEntries(...)`에 맡깁니다.
