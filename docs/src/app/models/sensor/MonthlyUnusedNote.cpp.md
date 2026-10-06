# `src/app/models/sensor/MonthlyUnusedNote.cpp`

<a id="responsibility"></a>

## 책임

`UnusedNoteSensorSupport` 주위에 월간 비활성 래퍼를 구현합니다.

<a id="window-definition"></a>

## 창 정의

- 새로 고침당 하나의 UTC 참조 시간을 캡처합니다.
- `referenceUtc.addMonths(-1)`를 월별 기준값으로 사용하여 창이 고정된 `30` -일 근사값이 아니라 달력 월 의미를 따르도록 합니다.
