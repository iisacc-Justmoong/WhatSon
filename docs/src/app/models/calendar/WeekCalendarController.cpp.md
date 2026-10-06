# `src/app/models/calendar/WeekCalendarController.cpp`

<a id="implementation-notes"></a>

## 구현 노트
- 이제 주간 새로 고침이 `ICalendarBoardStore::entriesChanged`를 관찰합니다.
- 컨트롤러는 여전히 정규화된 주 앵커를 소유합니다.
- 컨트롤러는 이제 지연 로딩하는 가로 타임라인 창(`timelineDayModels`)도 소유하며, QML 측에 두 번째 `ListModel`을 두지 않고 해당 창의 초기화·앞에 추가·뒤에 추가·잘라내기·새로 고침을 수행할 수 있다.
- 타임라인 데이 모델은 이제 이미 `entries`, `eventCount`, `taskCount`, `weekdayLabel` 및 `dateLabel`를 포함하고 있으므로, 주 페이지는 자체적으로 날짜 메타데이터나 캐시 항목 목록을 재구성하지 않습니다.
- QML 레이어는 이제 3-열 뷰포트의 포커스된 중간 날짜에서 `displayedWeekStartIso`를 동기화하여, `Today`가 실제 현재 날짜에 도달하면서 주간 수준 앵커를 유지할 수 있습니다.
- `requestWeekView(...)`는 이제 훅/로그 전용이며, 실제 주간 재조립은 `setDisplayedWeekStartIso(...)`, `setCalendarBoardStore(...)`, 그리고 보드 진입 변이를 뒤에 두어 페이지 열기와 이전/다음 탐색이 두 번 재구축되지 않도록 합니다.
- `trimTimelineWindow(...)`는 `QVariantList::size()`를 명시적인 `int` 윈도우 카운트로 정규화한 뒤 `std::min(...)`를 호출하여, Qt 6 `qsizetype` 컨테이너 API가 타임라인 트림 컴파일 경로를 깨는 것을 방지합니다.

<a id="regression-checks"></a>

## 회귀 수표
- `trimTimelineWindow(...)`는 `QVariantList::size()`가 Qt 6에서 `qsizetype`로 확인될 때 계속 컴파일해야 합니다.
