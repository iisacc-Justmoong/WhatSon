# `src/app/models/calendar/WeekCalendarController.hpp`

<a id="role"></a>

## 역할
`WeekCalendarController`는 표준 7일 주간 투영과 주간 페이지에서 사용하는 지연 로딩 가로 타임라인 날짜 창을 모두 노출한다.

<a id="interface-alignment"></a>

## 인터페이스 정렬
- 이제 캘린더 보드 삽입은 `ICalendarBoardStore`에 따라 달라집니다.
- `displayedWeekStartIso`는 QML 표면이 이제 하나의 연속된 수평 스캐폴드에서 3의 가시적인 낮 열을 렌더링하더라도 여전히 정규 주간 앵커로 남아 있습니다.
- 컨트롤러는 이제 `timelineDayModels` 를 통해 그 스캐폴드의 타임라인-일 모델 창을 소유하며, `initializeTimelineWindow` , `prependTimelineDates` , `appendTimelineDates` , `trimTimelineWindow` 와 같은 증분 창 변형 방법을 통해 QML 는 더 이상 두 번째 임시 날짜 모델 계층 구조를 유지하지 않습니다.
- 주간 앵커는 이제 뷰포트의 포커스된 중간 날짜에서 동기화되며, 3-열 표면의 왼쪽 가장자리에서는 동기화되지 않습니다.
- QML의 현재 주 시각적 강조는 월요일 시작 주에 대해 계산되며, 주간 경로에서 사용되는 동일한 주간 고정 기대치와 일치합니다.
